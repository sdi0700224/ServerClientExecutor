#include "Commander.h"
#include "PipeManager.h"
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>


Commander::Commander() : PipeHandler(ReadPipePath, WritePipePath), ServerPid(-1)
{
    if (IsFileExist(PidFileName.c_str()))
    {
        ifstream file(PidFileName);
        int pid;
        file >> pid;
        if (!file)
        {
            cerr << "Failed to read PID from file.";
            return;
        }
        ServerPid = pid;
    }
    else
    {
        StartServer();
    }
}

void Commander::StartServer()
{
    cout << "JobExecutorServer is not running. Starting it..." << endl;

    auto pid = fork();
    if (pid < 0)
    {
        cerr << "Fork failed! JobExecutorServer can not be started!" << endl;
        return;
    }
    else if (pid == 0)
    {
        execl("./jobExecutorServer", "./jobExecutorServer", (char *)NULL);
        cerr << "execl failed!" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        ServerPid = pid;
        sleep(1);
    }
}

Commander::~Commander()
{
}

void Commander::WaitServer()
{
    if (ServerPid == -1 ) { return; }

    int status;
    waitpid(ServerPid, &status, 0);
}

void Commander::IssueJob(const string& job)
{
    auto command = "issueJob " + job;
    SendCommand(command);
    ReceiveResponse();
}

void Commander::SetConcurrency(int level)
{
    auto command = "setConcurrency " + to_string(level);
    SendCommand(command);
}

void Commander::StopJob(const string& jobId)
{
    auto command = "stop " + jobId;
    SendCommand(command);
    ReceiveResponse();
}

void Commander::PollJobs(const string& type)
{
    auto command = "poll " + type;
    SendCommand(command);
    ReceiveResponse();
}

void Commander::ExitServer()
{
    SendCommand("exit");
    ReceiveResponse();
    WaitServer();
}

void Commander::SendCommand(const string& command)
{
    if (ServerPid == -1 )
    {
        cout << "Command cound not be sent, server is not running.." << endl;
        return;
    }

    kill(ServerPid, SIGUSR1);
    PipeHandler.WriteToCommonPipe(WritePipePath + " " + ReadPipePath + "\n"); // Reverse pipes in the other end
    PipeHandler.WriteToPipe(command);
}

string Commander::ReceiveResponse()
{
    auto response = PipeHandler.ReadFromPipe();
    cout << response << endl;
    return response;
}

bool Commander::IsFileExist(const char *fileName)
{
    struct stat buffer;
    return (stat(fileName, &buffer) == 0);
}
