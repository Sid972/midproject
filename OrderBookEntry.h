#pragma once

#include <string>
/**
 * Enum for the type of orderbook entry.
 */
enum class OrderBookType{bid, ask, unknown, asksale, bidsale};
/**
 * Represents a single entry in the order book.
 * Fields:
 *   - price:  price per unit (double)
 *   - amount: amount of currency (double)
 *   - timestamp: “YYYY/MM/DD HH:MM:SS.ffffff” format string
 *   - product: e.g. "ETH/USDT"
 *   - orderType: bid or ask (or sale versions for matched orders)
 *   - username: who placed it (e.g. "dataset" or "simuser")
 */
class OrderBookEntry
{
    public:
    
    // Primary constructor
        OrderBookEntry( double _price, 
                        double _amount, 
                        std::string _timestamp, 
                        std::string _product, 
                        OrderBookType _orderType, 
                        std::string username = "dataset");
    /**
         * Convert string "ask" / "bid" / etc. into our enum.
         */
        static OrderBookType stringToOrderBookType(std::string s);
    // Sorting helpers (not used directly here, but available if needed):
        static bool compareByTimestamp(OrderBookEntry& e1, OrderBookEntry& e2)
        {
            return e1.timestamp < e2.timestamp;
        }  
        static bool compareByPriceAsc(OrderBookEntry& e1, OrderBookEntry& e2)
        {
            return e1.price < e2.price;
        }
         static bool compareByPriceDesc(OrderBookEntry& e1, OrderBookEntry& e2)
        {
            return e1.price > e2.price;
        }

        double price;
        double amount;
        std::string timestamp;
        std::string product;
        OrderBookType orderType;
        std::string username;
};
