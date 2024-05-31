#include "orderbook.h"
#include "logger.h"
#include <iomanip>

OrderBook::OrderBook() : running(false), processedOrders(0) {}

OrderBook::~OrderBook() {
    stop();
}

void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(order.type == BUY || order.type == MARKET || order.type == STOP ? buyMutex : sellMutex);
    Logger::getInstance().log("Adding order: " + std::to_string(order.id));
    if (order.type == BUY || order.type == MARKET || order.type == STOP) {
        buyOrders[order.price].push(order);
    }
    else {
        sellOrders[order.price].push(order);
    }
    cv.notify_one();
}

void OrderBook::cancelOrder(int orderId) {
    Logger::getInstance().log("Cancelling order: " + std::to_string(orderId));
    // Simplified cancellation logic
}

void OrderBook::start() {
    running = true;
    startTime = std::chrono::high_resolution_clock::now();
    int threadCount = std::thread::hardware_concurrency();
    for (int i = 0; i < threadCount; ++i) {
        workerThreads.emplace_back(&OrderBook::processOrders, this);
    }
}

void OrderBook::stop() {
    running = false;
    cv.notify_all();
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    endTime = std::chrono::high_resolution_clock::now();
}

void OrderBook::matchOrders() {
    while (running) {
        std::unique_lock<std::mutex> lock(bookMutex);
        cv.wait(lock, [this] { return !running || (!buyOrders.empty() && !sellOrders.empty()); });

        if (!running) {
            break;
        }

        while (!buyOrders.empty() && !sellOrders.empty()) {
            auto buyIt = buyOrders.rbegin();
            auto sellIt = sellOrders.begin();

            if (buyIt->first >= sellIt->first) {
                auto& buyQueue = buyIt->second;
                auto& sellQueue = sellIt->second;

                Order& buyOrder = buyQueue.front();
                Order& sellOrder = sellQueue.front();

                executeOrder(buyOrder);
                executeOrder(sellOrder);

                int matchedQuantity = std::min(buyOrder.quantity, sellOrder.quantity);
                buyOrder.quantity -= matchedQuantity;
                sellOrder.quantity -= matchedQuantity;

                // Log the trade execution with buy and sell prices
                logTradeExecution(buyOrder.id, sellOrder.id, buyOrder.price, sellOrder.price, matchedQuantity);

                if (buyOrder.quantity == 0) {
                    buyOrder.status = FILLED;
                    buyQueue.pop();
                }
                else {
                    buyOrder.status = PARTIALLY_FILLED;
                }

                if (sellOrder.quantity == 0) {
                    sellOrder.status = FILLED;
                    sellQueue.pop();
                }
                else {
                    sellOrder.status = PARTIALLY_FILLED;
                }

                Logger::getInstance().log("Matched order: " + std::to_string(buyOrder.id) +
                    " with order: " + std::to_string(sellOrder.id));
                processedOrders++;
                matchedOrders.push_back(buyOrder);
                matchedOrders.push_back(sellOrder);

                if (buyQueue.empty()) {
                    buyOrders.erase(buyIt->first);
                }

                if (sellQueue.empty()) {
                    sellOrders.erase(sellIt->first);
                }
            }
            else {
                break;
            }
        }
    }
}

void OrderBook::executeOrder(Order& order) {
    if (order.type == MARKET) {
        if (order.type == BUY && !sellOrders.empty()) {
            order.price = sellOrders.begin()->first;
        }
        else if (order.type == SELL && !buyOrders.empty()) {
            order.price = buyOrders.rbegin()->first;
        }
    }
    else if (order.type == STOP) {
        if (order.type == BUY && !sellOrders.empty() && sellOrders.begin()->first <= order.price) {
            order.type = MARKET;
            order.price = sellOrders.begin()->first;
        }
        else if (order.type == SELL && !buyOrders.empty() && buyOrders.rbegin()->first >= order.price) {
            order.type = MARKET;
            order.price = buyOrders.rbegin()->first;
        }
    }
}

void OrderBook::processOrders() {
    while (running) {
        std::vector<Order> batch;
        {
            std::unique_lock<std::mutex> lock(bookMutex);
            cv.wait(lock, [this] { return !running || (!buyOrders.empty() && !sellOrders.empty()); });

            if (!running) {
                break;
            }

            std::lock_guard<std::mutex> buyLock(buyMutex);
            std::lock_guard<std::mutex> sellLock(sellMutex);

            // Batch processing
            auto buyIt = buyOrders.rbegin();
            auto sellIt = sellOrders.begin();

            while (buyIt != buyOrders.rend() && sellIt != sellOrders.end() && buyIt->first >= sellIt->first) {
                batch.push_back(buyIt->second.front());
                buyIt->second.pop();
                if (buyIt->second.empty()) {
                    buyIt = std::map<double, std::queue<Order>>::reverse_iterator(buyOrders.erase(std::next(buyIt).base()));
                }

                batch.push_back(sellIt->second.front());
                sellIt->second.pop();
                if (sellIt->second.empty()) {
                    sellIt = sellOrders.erase(sellIt);
                }
            }
        }

        processBatch(batch);
    }
}

void OrderBook::processBatch(std::vector<Order>& batch) {
    for (size_t i = 0; i < batch.size(); i += 2) {
        if (i + 1 < batch.size()) {
            Order& buyOrder = batch[i];
            Order& sellOrder = batch[i + 1];

            int matchedQuantity = std::min(buyOrder.quantity, sellOrder.quantity);
            buyOrder.quantity -= matchedQuantity;
            sellOrder.quantity -= matchedQuantity;

            // Log the trade execution with buy and sell prices
            logTradeExecution(buyOrder.id, sellOrder.id, buyOrder.price, sellOrder.price, matchedQuantity);

            if (buyOrder.quantity == 0) {
                buyOrder.status = FILLED;
            }
            else {
                buyOrder.status = PARTIALLY_FILLED;
            }

            if (sellOrder.quantity == 0) {
                sellOrder.status = FILLED;
            }
            else {
                sellOrder.status = PARTIALLY_FILLED;
            }

            Logger::getInstance().log("Matched order: " + std::to_string(buyOrder.id) +
                " with order: " + std::to_string(sellOrder.id));
            processedOrders++;
            matchedOrders.push_back(buyOrder);
            matchedOrders.push_back(sellOrder);
        }
    }
}

void OrderBook::logTradeExecution(int buyOrderId, int sellOrderId, double buyPrice, double sellPrice, int quantity) {
    executedTrades.emplace_back(buyOrderId, sellOrderId, buyPrice, sellPrice, quantity);
}

void OrderBook::printTradeExecutions() const {
    std::cout << "Trade Executions:" << std::endl;
    for (const auto& trade : executedTrades) {
        std::cout << "Buy Order ID: " << trade.buyOrderId
            << ", Sell Order ID: " << trade.sellOrderId
            << ", Buy Price: " << std::fixed << std::setprecision(3) << trade.buyPrice
            << ", Sell Price: " << std::fixed << std::setprecision(3) << trade.sellPrice
            << ", Quantity: " << trade.quantity << std::endl;
    }
}

int OrderBook::getProcessedOrderCount() const {
    return processedOrders;
}

double OrderBook::getElapsedTime() const {
    return std::chrono::duration<double>(endTime - startTime).count();
}

void OrderBook::printMatchedOrders() const {
    for (const auto& order : matchedOrders) {
        order.print();
    }
}

void OrderBook::printOrderBookDepth() const {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Order Book Depth:" << std::endl;
    std::cout << "Buy Orders:" << std::endl;
    for (const auto& entry : buyOrders) {
        std::cout << "Price: " << entry.first << ", Quantity: " << entry.second.size()
            << std::endl;
    }
    std::cout << "Sell Orders:" << std::endl;
    for (const auto& entry : sellOrders) {
        std::cout << "Price: " << entry.first << ", Quantity: " << entry.second.size()
            << std::endl;
    }
}

