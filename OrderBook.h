#pragma once
#include <string>
#include <vector>
#include<map>
#include "Candlestick.h"
#include "OrderBookEntry.h"
#include "CSVReader.h"

/**
 * Core “OrderBook” class that:
 *  1) Loads two CSV files of raw orders into a single `orders` vector
 *  2) Provides methods to filter and query orders by product, timestamp, and side
 *  3) Computes OHLC candlestick data
 *  4) Computes volume data
 *  5) Computes mean‐price data (per minute)
 *  6) Counts total trades per product
 */

class OrderBook
{
    public:
    /**
    * Construct by reading two CSV files (e.g. “20200317.csv” and “20200601.csv”).
    * Each CSV row becomes an OrderBookEntry. After loading, we sort ascending by timestamp.
    */

    //OrderBook(const std::string& filename);
    OrderBook(const std::string& file1,const std::string& file2);
    /** return vector of all know products in the dataset*/
    /**
     * Return a vector of all unique products seen across all orders (in no particular order).
     */
        std::vector<std::string> getKnownProducts();
    /**
    * Return a vector containing every order for:
    *   - given side (ask/bid)
    *   - matching product string
    *   - exactly matching timestamp string
    */
    /** return vector of Orders according to the sent filters*/
        std::vector<OrderBookEntry> getOrders(OrderBookType type, 
                                              std::string product, 
                                              std::string timestamp);
    /**
        * Return earliest timestamp in the orderbook (smallest lexicographically).
        */
        /** returns the earliest time in the orderbook*/
        std::string getEarliestTime();
    /**
 * Given a timestamp `t`, return the next‐higher timestamp in ascending order.
 * If `t` is highest, wrap back to the earliest.
 */
        std::string getNextTime(std::string timestamp);
    /**
     * Insert a new order (e.g. user bid/ask), keeping `orders` sorted by timestamp.
     */
    /**
    * TASK 4: Count total orders (“trades”) per product across all timestamps/sides.
    * Returns a map: product → count.
    */
    std::map<std::string, int> getTradesPerProduct();
    /**
      * TASK 2: Mean‐price data (per minute):
      *   - Group bids (or asks) by truncated minute (“HH:MM”)
      *   - Compute average price in that minute
      *   - Return vector of (minuteLabel, avgPrice)
      */
    std::vector<std::pair<std::string, double>> getMeanPriceData(OrderBookType type, const std::string& product);

        void insertOrder(OrderBookEntry& order);
    /**
        * Match asks to bids for the given product at the given timestamp.
        *   - Fetch all asks and all bids.
        *   - Sort asks ascending (lowest price first), bids descending (highest price first).
        *   - Walk through matching logic:
        *       If bid.price >= ask.price, we have a trade at ask.price.
        *       Create a “sale” OrderBookEntry with orderType = asksale or bidsale.
        *       Decrease amounts on either side as leftover orders.
        *   - Return vector of “sales” (matched trades).
        */
        std::vector<OrderBookEntry> matchAsksToBids(std::string product, std::string timestamp);
    /**
         * Return highest price among a vector of orders.
         */
        static double getHighPrice(std::vector<OrderBookEntry>& orders);
    /**
    * Return lowest price among a vector of orders.
    */
        static double getLowPrice(std::vector<OrderBookEntry>& orders);
        /** Compute OHLC candlesticks for one product & side over all timestamps */
        std::vector<Candlestick>
    /**
    * TASK 1: Compute OHLC candlesticks:
    * For every unique timestamp (collected from CSVReader::getAllTimestamps()):
    *   - Filter orders matching (side, product, timestamp)
    *   - Compute high = max(price), low = min(price)
    *   - Compute VWAP‐style close = ∑(price*amount) / ∑(amount)
    *   - Open = previous close (or equal to close for first candle)
    *   - Append a new Candlestick(ts, open, high, low, close)
    */
        getCandlestickData(OrderBookType side, const std::string& product);
        std::vector<std::pair<std::string,double>>
    /**
    * TASK 3a (part): Volume data:
    * For each timestamp, sum up total `amount` of orders (side, product).
    * Return vector of (timestamp, totalAmount).
    */
        getVolumeData(OrderBookType side, const std::string& product);

    private:
        std::vector<OrderBookEntry> orders;// All loaded ask/bid entries, sorted by timestamp


};
