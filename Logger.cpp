#include "logger.h"

Logger::Logger() : running(false) {
    logFile.open("trading_engine.log", std::ios::app);
}

Logger::~Logger() {
    stop();
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(message);
    }
    cv.notify_one();
}

void Logger::start() {
    running = true;
    logThread = std::thread(&Logger::processLogs, this);
}

void Logger::stop() {
    running = false;
    cv.notify_all();
    if (logThread.joinable()) {
        logThread.join();
    }
}

void Logger::processLogs() {
    while (running) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !logQueue.empty() || !running; });

        while (!logQueue.empty()) {
            logFile << logQueue.front() << std::endl;
            logQueue.pop();
        }
    }
}
