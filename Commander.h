#pragma once
#include "PipeManager.h"
#include <string>
using namespace std;

class Commander {
private:
    const string PidFileName = "jobExecutorServer.txt";
    const string ReadPipePath = "/tmp/pipeA";
    const string WritePipePath = "/tmp/pipeB";
    
    PipeManager PipeHandler;
    int ServerPid;

    void StartServer();
    void WaitServer();
    string ReceiveResponse();
    void SendCommand(const string& command);
    bool IsFileExist(const char *fileName);

public:
    Commander();
    ~Commander();

    void IssueJob(const string& job);
    void SetConcurrency(int level);
    void StopJob(const string& jobId);
    void PollJobs(const string& type);
    void ExitServer();
};