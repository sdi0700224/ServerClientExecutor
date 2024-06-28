#include "Server.h"
#include <iostream>
#include <string>
#include <csignal>
#include <unistd.h>
#include <atomic>
#include <mutex>
using namespace std;

int main(int argc, char* argv[])
{
    Server server;
    server.Run();

    return EXIT_SUCCESS;
}
