#include "PipeManager.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>

int PipeManager::CommonFd = -1;
string PipeManager::CommonPipePath = "/tmp/pipeCommon";

PipeManager::PipeManager(const string& readPipe, const string& writePipe)
    : ReadFd(-1), WriteFd(-1), ReadPipePath(readPipe), WritePipePath(writePipe)
{
    CreateCommonPipe();
    if (mkfifo(ReadPipePath.c_str(), 0666) == -1 && errno != EEXIST)
    {
        cerr << "Constructor - Failed to create read pipe: " << strerror(errno) << endl;
    }
    if (mkfifo(WritePipePath.c_str(), 0666) == -1 && errno != EEXIST)
    {
        cerr << "Constructor - Failed to create write pipe: " << strerror(errno) << endl;
    }
}

void PipeManager::CreateCommonPipe()
{
    if (mkfifo(PipeManager::CommonPipePath.c_str(), 0666) == -1 && errno != EEXIST)
    {
        cerr << "Constructor - Failed to create common pipe: " << strerror(errno) << endl;
    }
}

PipeManager::~PipeManager()
{
    CloseCommonPipe();
    CloseReadPipe();
    CloseWritePipe();
}

void PipeManager::OpenCommonPipe(bool read)
{
    CommonFd = read ?   open(CommonPipePath.c_str(), O_RDONLY) :
                        open(CommonPipePath.c_str(), O_WRONLY);
    if (CommonFd == -1)
    {
        cerr << "CommonReadPipe - Failed to open common pipe for " << (read ? "read" : "write") << ": " << strerror(errno) << endl;
    }
}

void PipeManager::OpenReadPipe()
{
    ReadFd = open(ReadPipePath.c_str(), O_RDONLY);
    if (ReadFd == -1)
    {
        cerr << "OpenReadPipe - Failed to open read pipe: " << strerror(errno) << endl;
    }
}

void PipeManager::OpenWritePipe()
{
    WriteFd = open(WritePipePath.c_str(), O_WRONLY);
    if (WriteFd == -1)
    {
        cerr << "OpenWritePipe - Failed to open write pipe: " << strerror(errno) << endl;
    }
}

void PipeManager::CloseCommonPipe()
{
    if (CommonFd != -1)
    {
        close(CommonFd);
        CommonFd = -1;
    }
}

void PipeManager::CloseReadPipe()
{
    if (ReadFd != -1)
    {
        close(ReadFd);
        ReadFd = -1;
    }
}

void PipeManager::CloseWritePipe()
{
    if (WriteFd != -1)
    {
        close(WriteFd);
        WriteFd = -1;
    }
}

void PipeManager::WriteToPipe(const string& message)
{
    OpenWritePipe();
    const char* data = message.c_str();
    size_t total = message.size();
    ssize_t bytes_written = 0;

    while (total > 0)
    {
        bytes_written = write(WriteFd, data, total);
        if (bytes_written == -1)
        {
            if (errno == EINTR)
            {
                continue; // Retry if interrupted by signal
            }
            else
            {
                cerr << "WriteToPipe - Failed to write to pipe: " << strerror(errno) << endl;
                break;
            }
        }
        data += bytes_written;
        total -= bytes_written;
    }
    CloseWritePipe();
}

void PipeManager::WriteToCommonPipe(const string& message)
{
    OpenCommonPipe(false);
    const char* data = message.c_str();
    size_t total = message.size();
    ssize_t bytes_written = 0;

    while (total > 0)
    {
        bytes_written = write(CommonFd, data, total);
        if (bytes_written == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                cerr << "WriteToPipe - Failed to write to pipe: " << strerror(errno) << endl;
                break;
            }
        }
        data += bytes_written;
        total -= bytes_written;
    }
    CloseCommonPipe();
}

string PipeManager::ReadFromPipe()
{
    OpenReadPipe();
    char buffer[256];
    string result;
    ssize_t bytesRead = 0;

    while ((bytesRead = read(ReadFd, buffer, sizeof(buffer) - 1)) != 0)
    {
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            result.append(buffer);
        }
        else if (bytesRead == -1)
        {
            if (errno == EAGAIN)
            {
                break; // No data available in non-blocking mode
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else
            {
                cerr << "ReadFromPipe - Failed to read from pipe: " << strerror(errno) << endl;
                break;
            }
        }
    }
    CloseReadPipe();
    return result;
}

string PipeManager::ReadFromCommonPipe()
{
    OpenCommonPipe(true);
    char buffer[256];
    string result;
    ssize_t bytesRead = 0;

    while ((bytesRead = read(CommonFd, buffer, sizeof(buffer) - 1)) != 0)
    {
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            result.append(buffer);
        }
        else if (bytesRead == -1)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else
            {
                cerr << "ReadFromPipe - Failed to read from pipe: " << strerror(errno) << endl;
                break;
            }
        }
    }
    CloseCommonPipe();
    return result;
}
