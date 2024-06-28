#pragma once
#include <string>
using namespace std;

struct Job {
    string JobID;
    string Command;
    int QueuePosition;
    pid_t Pid;
};