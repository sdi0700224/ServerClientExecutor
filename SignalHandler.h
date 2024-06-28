#pragma once
#include <atomic>
#include <functional>
#include <mutex>
using namespace std;

class SignalHandler
{
public:
    SignalHandler();
    void SetupSignalHandlers();
    void HandleSignals();
    void SetSigusr1Handler(std::function<void()> handler);
    void SetSigchldHandler(std::function<void()> handler);

private:
    static std::atomic<bool> Sigusr1Received;
    static std::atomic<bool> ChildFinished;
    static SignalHandler* Instance;

    function<void()> customSigusr1Handler;
    function<void()> customSigchldHandler;

    static void Sigusr1Handle(int signum);
    static void SigchldHandle(int signum);
};
