#include "order.h"

bool Order::isValid() const {
    return price > 0 && quantity > 0;
}

void Order::print() const {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Order ID: " << id << ", Type: "
        << (type == BUY ? "BUY" : (type == SELL ? "SELL" :
            (type == MARKET ? "MARKET" : (type == LIMIT ? "LIMIT" : "STOP"))))
        << ", Price: " << price << ", Quantity: " << quantity
        << ", Status: " << status << std::endl;
}
