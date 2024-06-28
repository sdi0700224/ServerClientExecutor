#include "Server.h"
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

Server::Server() : CurrentConcurrencyLevel(1), IsExitRequested(false), JobCounter(0)
{
    CreateFileWithPid(PidFileName);
    SignalHandle.SetupSignalHandlers();
    SignalHandle.SetSigusr1Handler([this]() { this->CustomSigusr1Handler(); });
    SignalHandle.SetSigchldHandler([this]() { this->CustomSigchldHandler(); });
}

Server::~Server()
{
    RemoveFileWithPid();
}

void Server::Run()
{
    cout << "Server started." << endl;
    
    while (!IsExitRequested)
    {
        pause();
        SignalHandle.HandleSignals();
        HandleJobExecution();
    }
}

void Server::Stop()
{
    cout << "Server stopped." << endl;
    IsExitRequested = true;
}

void Server::CreateFileWithPid(const string& filename)
{
    ofstream outfile(filename);
    if (outfile.is_open())
    {
        auto pid = getpid();
        outfile << pid << endl;
        outfile.close();
    }
    else
    {
        cerr << "Failed to create file: " << filename << endl;
    }
}

void Server::RemoveFileWithPid()
{
    if (remove(PidFileName.c_str()) != 0)
    {
        cerr << "Error deleting file " << PidFileName << "." << endl;
    }
}

void Server::CustomSigusr1Handler()
{
    auto pipeNames = PipeManager::ReadFromCommonPipe();
    string readPipe, writePipe;
    istringstream stream(pipeNames);
    stream >> readPipe >> writePipe;
    PipeManager pipeHandler(readPipe, writePipe);
    auto command = ReadCommand(pipeHandler);

    istringstream iss(command);
    string cmd;
    iss >> cmd;

    if (cmd == "issueJob")
    {
        string jobCommand;
        getline(iss, jobCommand);
        auto jobInfo = EnqueueJob(jobCommand);
        pipeHandler.WriteToPipe(jobInfo);
    }
    else if (cmd == "setConcurrency")
    {
        int level;
        iss >> level;
        SetConcurrencyLevel(level);
    }
    else if (cmd == "stop")
    {
        string jobId;
        iss >> jobId;
        string message = TerminateJob(jobId);
        pipeHandler.WriteToPipe(message);
    }
    else if (cmd == "poll")
    {
        string type;
        iss >> type;
        if (type == "running")
        {
            PollRunningJobs(pipeHandler);
        }
        else if (type == "queued")
        {
            PollQueuedJobs(pipeHandler);
        }
    }
    else if (cmd == "exit")
    {
        Stop();
        pipeHandler.WriteToPipe("jobExecutorServer terminated");
    }
    else
    {
        cerr << "Unknown command received: " << cmd << endl;
    }
}

void Server::CustomSigchldHandler()
{
    CheckForCompletedJobs();
}

string Server::ReadCommand(PipeManager& pipeHandler)
{
    return pipeHandler.ReadFromPipe();
}

void Server::SetConcurrencyLevel(int newLevel)
{
    if (newLevel > 0)
    {
        CurrentConcurrencyLevel = newLevel;
    }
}

string Server::EnqueueJob(const string& jobCommand)
{
    Job newJob{ "job_" + to_string(++JobCounter), jobCommand, static_cast<int>(QueuedJobs.size() + 1), -1 };
    QueuedJobs.push_back(newJob);
    return newJob.JobID + "," + newJob.Command + "," + to_string(newJob.QueuePosition);
}

string Server::TerminateJob(const string& jobId)
{
    auto it = find_if(RunningJobs.begin(), RunningJobs.end(), [&](const Job& job)
    {
        return job.JobID == jobId;
    });

    if (it != RunningJobs.end())
    {
        kill(it->Pid, SIGTERM);
        string message = it->JobID + " terminated";
        RunningJobs.erase(it);
        return message;
    }

    auto queuedIt = find_if(QueuedJobs.begin(), QueuedJobs.end(), [&](const Job& job)
    {
        return job.JobID == jobId;
    });

    if (queuedIt != QueuedJobs.end())
    {
        string message = queuedIt->JobID + " removed";
        QueuedJobs.erase(queuedIt);
        return message;
    }

    return "Job not found";
}

void Server::PollRunningJobs(PipeManager& pipeHandler)
{
    stringstream response;
    for (const auto& job : RunningJobs)
    {
        response << job.JobID << "," << job.Command << "," << job.QueuePosition << "\n";
    }
    pipeHandler.WriteToPipe(response.str());
}

void Server::PollQueuedJobs(PipeManager& pipeHandler)
{
    stringstream response;
    for (const auto& job : QueuedJobs)
    {
        response << job.JobID << "," << job.Command << "," << job.QueuePosition << "\n";
    }
    pipeHandler.WriteToPipe(response.str());
}

void Server::HandleJobExecution()
{
    // Start new jobs if there is space available
    while ((int)RunningJobs.size() < CurrentConcurrencyLevel && !QueuedJobs.empty())
    {
        Job job = QueuedJobs.front();
        QueuedJobs.erase(QueuedJobs.begin());
        pid_t pid = fork();

        if (pid < 0)
        {
            cerr << "Failed to fork for job: " << job.Command << endl;
            QueuedJobs.insert(QueuedJobs.begin(), job); // Requeue the job
            break;
        }
        else if (pid == 0)
        {
            execl("/bin/sh", "sh", "-c", job.Command.c_str(), (char*)nullptr);
            cerr << "Failed to exec for job: " << job.Command << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            job.Pid = pid;
            RunningJobs.push_back(job);
        }
    }
}

void Server::CheckForCompletedJobs()
{
    int status = -1;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        auto it = find_if(RunningJobs.begin(), RunningJobs.end(), [pid](const Job& job)
        {
            return job.Pid == pid;
        });

        if (it != RunningJobs.end())
        {
            cout << "Job " << it->JobID << " completed with status " << status << endl;
            RunningJobs.erase(it);
        }
    }
}