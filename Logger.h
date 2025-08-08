#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <string>

class Logger {
public:
    static Logger& getInstance();
    void log(const std::string& message);
    void start();
    void stop();
    void setEnabled(bool isEnabled);

private:
    Logger();
    ~Logger();
    std::ofstream logFile;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::string> logQueue;
    std::thread logThread;
    std::atomic<bool> running;
    std::atomic<bool> enabled{true};

    void processLogs();
};
