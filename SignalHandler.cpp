#include "SignalHandler.h"
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

atomic<bool> SignalHandler::Sigusr1Received(false);
atomic<bool> SignalHandler::ChildFinished(false);
SignalHandler* SignalHandler::Instance = nullptr;

SignalHandler::SignalHandler()
{
    Instance = this;
}

void SignalHandler::SetupSignalHandlers()
{
    struct sigaction sa;

    sa.sa_handler = Sigusr1Handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, nullptr) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = SigchldHandle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, nullptr) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void SignalHandler::Sigusr1Handle(int signum)
{
    Sigusr1Received.store(true);
}

void SignalHandler::SigchldHandle(int signum)
{
    ChildFinished.store(true);
}

void SignalHandler::HandleSignals()
{
    if (Sigusr1Received.exchange(false))
    {
        if (customSigusr1Handler)
        {
            customSigusr1Handler();
        }
    }

    if (ChildFinished.exchange(false))
    {
        if (customSigchldHandler)
        {
            customSigchldHandler();
        }
    }
}

void SignalHandler::SetSigusr1Handler(function<void()> handler)
{
    customSigusr1Handler = handler;
}

void SignalHandler::SetSigchldHandler(function<void()> handler)
{
    customSigchldHandler = handler;
}
