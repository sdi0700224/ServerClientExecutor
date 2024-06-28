#include "Commander.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <command> [arguments]" << endl;
        return EXIT_FAILURE;
    }

    Commander commander;
    string command = argv[1];

    if (command == "issueJob" && argc >= 3)
    {
        string job;
        for (int i = 2; i < argc; ++i)
        {
            job += string(argv[i]) + " ";
        }
        commander.IssueJob(job);
    }
    else if (command == "setConcurrency" && argc == 3)
    {
        auto level = stoi(argv[2]);
        commander.SetConcurrency(level);
    }
    else if (command == "stop" && argc == 3)
    {
        string jobId = argv[2];
        commander.StopJob(jobId);
    }
    else if (command == "poll" && argc == 3)
    {
        string type = argv[2];
        commander.PollJobs(type);
    }
    else if (command == "exit" && argc == 2)
    {
        commander.ExitServer();
    }
    else
    {
        cerr << "Invalid command or wrong number of arguments." << endl;
        cerr << "Usage examples:" << endl;
        cerr << argv[0] << " issueJob <command>" << endl;
        cerr << argv[0] << " setConcurrency <level>" << endl;
        cerr << argv[0] << " stop <jobId>" << endl;
        cerr << argv[0] << " poll [running|queued]" << endl;
        cerr << argv[0] << " exit" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
