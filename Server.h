#pragma once
#include "SignalHandler.h"
#include "PipeManager.h"
#include "Job.h"
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

class Server {
private:
    const string PidFileName = "jobExecutorServer.txt";

    SignalHandler SignalHandle;
    int CurrentConcurrencyLevel;
    vector<Job> RunningJobs;
    vector<Job> QueuedJobs;
    bool IsExitRequested;
    int JobCounter;

    void CustomSigusr1Handler();
    void CustomSigchldHandler();
    void CreateFileWithPid(const string& filename);
    void RemoveFileWithPid();
    string ReadCommand(PipeManager& pipeHandler);
    void SetConcurrencyLevel(int newLevel);
    string EnqueueJob(const string& jobCommand);
    string TerminateJob(const string& jobId);
    void PollRunningJobs(PipeManager& pipeHandler);
    void PollQueuedJobs(PipeManager& pipeHandler);
    void HandleJobExecution();
    void CheckForCompletedJobs();
    void Stop();

public:
    Server();
    ~Server();

    void Run();
};
