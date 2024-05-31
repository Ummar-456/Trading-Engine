#pragma once

#include <map>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include "order.h"

struct Trade {
    int buyOrderId;
    int sellOrderId;
    double buyPrice;
    double sellPrice;
    int quantity;

    Trade(int buyOrderId, int sellOrderId, double buyPrice, double sellPrice, int quantity)
        : buyOrderId(buyOrderId), sellOrderId(sellOrderId), buyPrice(buyPrice), sellPrice(sellPrice), quantity(quantity) {}
};

class OrderBook {
public:
    OrderBook();
    ~OrderBook();
    void addOrder(const Order& order);
    void cancelOrder(int orderId);
    void start();
    void stop();
    int getProcessedOrderCount() const;
    double getElapsedTime() const;
    void printMatchedOrders() const;
    void printOrderBookDepth() const;
    void printTradeExecutions() const;

private:
    std::map<double, std::queue<Order>> buyOrders;
    std::map<double, std::queue<Order>> sellOrders;
    std::vector<Order> matchedOrders;
    std::vector<Trade> executedTrades;
    std::mutex buyMutex, sellMutex, bookMutex;
    std::condition_variable cv;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> running;
    std::atomic<int> processedOrders;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

    void matchOrders();
    void processOrders();
    void executeOrder(Order& order);
    void processBatch(std::vector<Order>& batch);
    void logTradeExecution(int buyOrderId, int sellOrderId, double buyPrice, double sellPrice, int quantity);
};
