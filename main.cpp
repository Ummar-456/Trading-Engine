#include "orderbook.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>

// Function to read orders from a CSV file
std::vector<Order> readOrdersFromCSV(const std::string& filename) {
    std::vector<Order> orders;
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
        std::istringstream ss(line);
        std::getline(ss, date, ',');
        if (!(ss >> price)) {
            std::cerr << "Error reading price from line: " << line << std::endl;
            continue; // Skip lines with errors
        }

        // Alternate between BUY and SELL orders 
        OrderType type = (id % 2 == 0) ? BUY : SELL;
        orders.emplace_back(id, type, price, quantity);
        id++;
    }

    return orders;
}

int main() {
    try {
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
        double ordersPerSecond = processedOrders / elapsedTime;

        std::cout << "Processed Orders: " << processedOrders << std::endl;
        std::cout << "Elapsed Time: " << elapsedTime << " seconds" << std::endl;
        std::cout << "Orders Per Second: " << ordersPerSecond << std::endl;

        std::cout << "\nMatched Orders:\n";
        orderBook.printMatchedOrders();

        std::cout << "\nOrder Book Depth:\n";
        orderBook.printOrderBookDepth();

        std::cout << "\nTrade Executions:\n";
        orderBook.printTradeExecutions();

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
}
