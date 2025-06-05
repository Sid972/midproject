#pragma once

#include "OrderBookEntry.h"
#include <vector>
#include <string>
/**
 * CSVReader provides static methods to:
 *   1) Read a CSV file of raw orderbook lines into a vector<OrderBookEntry>
 *   2) Tokenize a single line by comma
 *   3) Convert tokens to an OrderBookEntry
 *   4) Gather all unique timestamps across multiple CSV files (for candlestick/volume loops)
 */

class CSVReader
{
    public:
     CSVReader();
    /**
        * Read an entire CSV (one order per line) into a vector<OrderBookEntry>.
        * Each line must have exactly 5 tokens: timestamp, product, orderType, price, amount.
        */
     static std::vector<OrderBookEntry> readCSV(std::string csvFile);
    /**
     * Tokenize a CSV line by `separator` character (usually comma).
     * Returns a vector of tokens (strings).
     */
     static std::vector<std::string> tokenise(std::string csvLine, char separator);
 /**
 * Convert (priceString, amountString, timestamp, product, orderType) into an OrderBookEntry.
 */
     static OrderBookEntry stringsToOBE(std::string price, 
                                        std::string amount, 
                                        std::string timestamp, 
                                        std::string product, 
                                        OrderBookType OrderBookType);
    /**
        * Return a sorted vector of every unique timestamp seen across all CSV files
        * listed in this function. We hardcode file names here (for simplicity).
        */
    // at the bottom of the public API
    /**
     * Return every unique timestamp in the CSV, sorted ascending.
     */
    static std::vector<std::string> getAllTimestamps();

    private:
     static OrderBookEntry stringsToOBE(std::vector<std::string> strings);
     
};