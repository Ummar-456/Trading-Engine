#pragma once

#include <iostream>
#include <iomanip>

enum OrderType { BUY, SELL, MARKET, LIMIT, STOP };
enum OrderStatus { NEW, PARTIALLY_FILLED, FILLED, CANCELLED, REJECTED };

struct Order {
    int id;
    OrderType type;
    double price;
    int quantity;
    OrderStatus status;

    Order(int id, OrderType type, double price, int quantity)
        : id(id), type(type), price(price), quantity(quantity), status(NEW) {}

    bool isValid() const;
    void print() const;
};
