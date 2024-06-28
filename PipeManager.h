#pragma once
#include <string>
using namespace std;

class PipeManager
{
private:
    static string CommonPipePath;
    static int CommonFd;
    int ReadFd;
    int WriteFd;
    string ReadPipePath;
    string WritePipePath;

    static void CreateCommonPipe();
    static void OpenCommonPipe(bool read);
    static void CloseCommonPipe();
    void OpenReadPipe();
    void OpenWritePipe();
    void CloseReadPipe();
    void CloseWritePipe();

public:
    PipeManager(const string& readPipe, const string& writePipe);
    ~PipeManager();

    static void WriteToCommonPipe(const string& message);
    static string ReadFromCommonPipe();
    void WriteToPipe(const string& message);
    string ReadFromPipe();
};