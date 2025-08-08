#include "Orderbook.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

// Function to read orders from a CSV file
std::vector<Order> readOrdersFromCSV(const std::string& filename) {
    std::vector<Order> orders;
    orders.reserve(6000); // pre-allocate for typical file size
    std::ifstream file(filename);
    std::string line, date;
    double price;
    int id = 1;
    int quantity = 10; // Example fixed quantity

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    // Skip the header
    std::getline(file, line);

    while (std::getline(file, line)) {
        const char* begin = line.c_str();
        const char* comma = static_cast<const char*>(memchr(begin, ',', line.size()));
        if (!comma) {
            continue;
        }
        char* endPtr = nullptr;
        price = std::strtod(comma + 1, &endPtr);
        if (endPtr == comma + 1) {
            continue;
        }

        OrderType type = (id % 2 == 0) ? BUY : SELL;
        orders.emplace_back(id, type, price, quantity);
        id++;
    }

    return orders;
}

int main(int argc, char** argv) {
    try {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);

        bool enableLog = false;
        bool printMatched = false;
        bool printDepth = false;
        bool printTrades = false;
        std::size_t topLevels = 5;
        std::size_t maxTrades = 50;

        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "--log") == 0) {
                enableLog = true;
            } else if (std::strcmp(argv[i], "--print-matched") == 0) {
                printMatched = true;
            } else if (std::strcmp(argv[i], "--print-depth") == 0) {
                printDepth = true;
            } else if (std::strcmp(argv[i], "--print-trades") == 0) {
                printTrades = true;
            } else if (std::strcmp(argv[i], "--top") == 0 && i + 1 < argc) {
                topLevels = static_cast<std::size_t>(std::strtoul(argv[++i], nullptr, 10));
            } else if (std::strncmp(argv[i], "--top=", 6) == 0) {
                topLevels = static_cast<std::size_t>(std::strtoul(argv[i] + 6, nullptr, 10));
            } else if (std::strcmp(argv[i], "--trades") == 0 && i + 1 < argc) {
                maxTrades = static_cast<std::size_t>(std::strtoul(argv[++i], nullptr, 10));
            } else if (std::strncmp(argv[i], "--trades=", 9) == 0) {
                maxTrades = static_cast<std::size_t>(std::strtoul(argv[i] + 9, nullptr, 10));
            }
        }

        Logger::getInstance().setEnabled(enableLog);
        Logger::getInstance().start();
        OrderBook orderBook;
        orderBook.start();

        // Read orders from the CSV file
        std::vector<Order> orders = readOrdersFromCSV("market_data.csv");

        // Add orders to the order book
        for (const auto& order : orders) {
            orderBook.addOrder(order);
        }

        // Stop the order book and logger
        orderBook.stop();
        Logger::getInstance().stop();

        // Print statistics and details about the processed orders
        int processedOrders = orderBook.getProcessedOrderCount();
        double elapsedTime = orderBook.getElapsedTime();
        double ordersPerSecond = elapsedTime > 0.0 ? (processedOrders / elapsedTime) : 0.0;

        std::cout << "Processed Orders: " << processedOrders << std::endl;
        std::cout << "Elapsed Time: " << elapsedTime << " seconds" << std::endl;
        std::cout << "Orders Per Second: " << ordersPerSecond << std::endl;

        if (printMatched) {
            std::cout << "\nMatched Orders:\n";
            orderBook.printMatchedOrders();
        }

        if (printDepth) {
            std::cout << "\nTop-of-book Depth:\n";
            orderBook.printOrderBookDepth(topLevels);
        }

        if (printTrades) {
            std::cout << "\nTrade Executions:\n";
            orderBook.printTradeExecutions(maxTrades);
        }

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
}
