#include "OrderBook.h"
#include "CSVReader.h"
#include "Candlestick.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <set>

/**
 * OrderBook:
 *   Loads, stores, and processes a collection of OrderBookEntry objects.
 *   Provides methods to:
 *     - Retrieve known products
 *     - Query orders by type/product/timestamp
 *     - Compute high/low prices
 *     - Generate candlestick OHLC data
 *     - Generate volume‐over‐time data
 *     - Find earliest/next timestamps
 *     - Insert new orders
 *     - Match asks to bids (trade execution)
 *     - Count trades per product
 *     - Compute average (mean) price per time bucket
 */

/**
 * Constructor (two‐file overload)
 * Reads two CSV files, merges their entries into a single vector, and sorts by timestamp.
 *
 * @param file1  Path to the first CSV (e.g., "20200317.csv")
 * @param file2  Path to the second CSV (e.g., "20200601.csv")
 *
 * Behavior:
 *   1. Calls CSVReader::readCSV(file1) and CSVReader::readCSV(file2) to get two vectors.
 *   2. Reserves enough space in `orders` to hold all entries from both vectors.
 *   3. Inserts all entries from the first vector, then all entries from the second.
 *   4. Sorts the combined `orders` by ascending timestamp so that time‐based methods work correctly.
 */
OrderBook::OrderBook(const std::string& file1,
                     const std::string& file2)
{
    // Read in all entries from the two CSV files
    auto march = CSVReader::readCSV(file1);
    auto june  = CSVReader::readCSV(file2);

    // Preallocate the `orders` vector for efficiency
    orders.reserve(march.size() + june.size());

    // Append all entries from March file, then from June file
    orders.insert(orders.end(), march.begin(), march.end());
    orders.insert(orders.end(), june.begin(),  june.end());

    // Sort by timestamp (lexicographically works because format is "YYYY/MM/DD HH:MM:SS")
    std::sort(orders.begin(), orders.end(),
        [](auto const &a, auto const &b) {
            return a.timestamp < b.timestamp;
        });
}

/**
 * getKnownProducts
 * Returns a vector of every distinct product string found in `orders`.
 *
 * Behavior:
 *   1. Iterates all OrderBookEntry in `orders` and inserts `e.product` into a map (or set) to dedupe.
 *   2. Flattens the map keys into a vector<string> and returns it.
 *
 * @return Vector<string> of unique product names (e.g., "BTC/USDT", "ETH/BTC", etc.)
 */
std::vector<std::string> OrderBook::getKnownProducts()
{
    std::vector<std::string> products;
    std::map<std::string,bool> prodMap; // maps product name to a dummy bool

    // Mark each product as "seen"
    for (OrderBookEntry& e : orders) {
        prodMap[e.product] = true;
    }

    // Extract keys (product names) into a vector
    for (auto const& pair : prodMap) {
        products.push_back(pair.first);
    }

    return products;
}

/**
 * getOrders
 * Retrieves all orders that match a given side, product, and exact timestamp.
 *
 * @param type       The OrderBookType (e.g., ask or bid)
 * @param product    The product string to filter on (e.g., "ETH/USDT")
 * @param timestamp  The exact timestamp string to filter on ("YYYY/MM/DD HH:MM:SS")
 *
 * @return A vector<OrderBookEntry> containing all matching orders; may be empty.
 *
 * Behavior:
 *   - Iterates through the entire `orders` vector.
 *   - If an entry’s orderType, product, and timestamp all match, add it to the result.
 */
std::vector<OrderBookEntry> OrderBook::getOrders(
    OrderBookType type,
    std::string product,
    std::string timestamp)
{
    std::vector<OrderBookEntry> orders_sub;
    for (OrderBookEntry& e : orders) {
        if (e.orderType == type &&
            e.product   == product &&
            e.timestamp == timestamp)
        {
            orders_sub.push_back(e);
        }
    }
    return orders_sub;
}

/**
 * getHighPrice
 * Finds the maximum price among a vector of OrderBookEntry.
 *
 * @param orders  A vector of OrderBookEntry (all assumed to have same side/product/timestamp)
 * @return The highest e.price among them. Caller must ensure `orders` is nonempty.
 *
 * Behavior:
 *   - Start with orders[0].price as initial max.
 *   - Iterate through all entries, update max if e.price > current max.
 */
double OrderBook::getHighPrice(std::vector<OrderBookEntry>& orders)
{
    // Assume orders is nonempty
    double maxPrice = orders[0].price;
    for (OrderBookEntry& e : orders) {
        if (e.price > maxPrice) {
            maxPrice = e.price;
        }
    }
    return maxPrice;
}

/**
 * getLowPrice
 * Finds the minimum price among a vector of OrderBookEntry.
 *
 * @param orders  A vector of OrderBookEntry (all assumed to have same side/product/timestamp)
 * @return The lowest e.price among them. Caller must ensure `orders` is nonempty.
 *
 * Behavior:
 *   - Start with orders[0].price as initial min.
 *   - Iterate through all entries, update min if e.price < current min.
 */
double OrderBook::getLowPrice(std::vector<OrderBookEntry>& orders)
{
    // Assume orders is nonempty
    double minPrice = orders[0].price;
    for (OrderBookEntry& e : orders) {
        if (e.price < minPrice) {
            minPrice = e.price;
        }
    }
    return minPrice;
}

/**
 * getCandlestickData
 * Generates OHLC (open, high, low, close) candlestick data for a given side and product.
 *
 * @param side     Which side to consider (OrderBookType::ask or ::bid)
 * @param product  The product to process (e.g., "ETH/USDT")
 *
 * @return A vector<Candlestick> containing one Candlestick per unique timestamp
 *         (that has at least one order on the given side/product).
 *
 * Behavior:
 *   1. Calls CSVReader::getAllTimestamps() to obtain a sorted vector<string> of all timestamps.
 *   2. Iterates each timestamp `ts` in ascending order:
 *        a. Calls getOrders(side, product, ts) to fetch all matching OrderBookEntry.
 *        b. If no entries, skip to next timestamp.
 *        c. Compute high = getHighPrice(entries), low = getLowPrice(entries).
 *        d. Compute VWAP‐style close = (∑(price*amount)) / (∑ amount).
 *        e. Determine open price:
 *             - If `candles` is empty (first candle), open = close.
 *             - Otherwise open = previous candle’s close.
 *        f. Append Candlestick(ts, open, high, low, close) to result.
 *        g. Set prevClose = close.
 *   3. Return the `candles` vector.
 */
std::vector<Candlestick> OrderBook::getCandlestickData(
    OrderBookType side,
    const std::string& product)
{
    std::vector<Candlestick> candles;

    // 1) Get all timestamps (sorted ascending, from CSVReader)
    auto times = CSVReader::getAllTimestamps();

    // Track the previous close price so that open = previousClose
    double prevClose = 0.0;

    // 2) For each timestamp in ascending order
    for (const auto& ts : times) {
        // 2a) Fetch all orders for this side/product/timestamp
        auto entries = getOrders(side, product, ts);
        if (entries.empty()) {
            continue;  // No orders at this timestamp; skip
        }

        // 2b) Compute high and low among the entries
        double high = getHighPrice(entries);
        double low  = getLowPrice(entries);

        // 2c) Compute VWAP-style close (weighted average by amount)
        double totVal = 0.0;
        double totAmt = 0.0;
        for (auto& e : entries) {
            totVal += e.price * e.amount;
            totAmt += e.amount;
        }
        double close = totVal / totAmt;

        // 2d) Compute open price: previous candle’s close, or equal to close if first candle
        double open = candles.empty() ? close : prevClose;

        // 2e) Add this new Candlestick to the vector
        candles.emplace_back(ts, open, high, low, close);

        // 2f) Update prevClose for next iteration
        prevClose = close;
    }

    return candles;
}

/**
 * getVolumeData
 * Builds a time‐series of total volume (sum of amounts) for each timestamp.
 *
 * @param side     Which side to consider (ask or bid)
 * @param product  The product (e.g., "ETH/USDT")
 *
 * @return A vector of (timestamp, totalAmount) pairs, where `totalAmount`
 *         is the sum of all e.amount for entries matching side/product at that timestamp.
 *
 * Behavior:
 *   1. Calls CSVReader::getAllTimestamps() to get a sorted list of all timestamps.
 *   2. For each timestamp `ts`:
 *        a. Calls getOrders(side, product, ts) to fetch matching entries.
 *        b. Sums up entry.amount for all these entries.
 *        c. Pushes (ts, totalAmt) into the result vector.
 *   3. Returns the vector of (timestamp, totalAmt) pairs.
 */
std::vector<std::pair<std::string, double>> OrderBook::getVolumeData(
    OrderBookType side,
    const std::string& product)
{
    std::vector<std::pair<std::string, double>> volumeSeries;

    // 1) Get all timestamps
    auto times = CSVReader::getAllTimestamps();

    // 2) Compute total volume per timestamp
    for (const auto& ts : times) {
        // 2a) Fetch all orders for this side/product/timestamp
        auto entries = getOrders(side, product, ts);

        // 2b) Sum up the amounts
        double totalAmt = 0.0;
        for (auto& e : entries) {
            totalAmt += e.amount;
        }

        // 2c) Record the (timestamp, totalAmt) pair
        volumeSeries.emplace_back(ts, totalAmt);
    }

    return volumeSeries;
}

/**
 * getEarliestTime
 * Returns the earliest timestamp among all orders.
 *
 * @return The timestamp string of the first (oldest) order in `orders`.
 *         Assumes `orders` is nonempty and already sorted by timestamp.
 */
std::string OrderBook::getEarliestTime()
{
    // Since `orders` was sorted in the constructor, the first element is earliest
    return orders[0].timestamp;
}

/**
 * getNextTime
 * Given a current timestamp, finds the next‐greater timestamp in `orders`.
 * If none exists (i.e., we were at the last timestamp), wrap around to the earliest.
 *
 * @param timestamp  The current timestamp string ("YYYY/MM/DD HH:MM:SS")
 * @return The next timestamp in ascending order, or the earliest timestamp if at the end.
 *
 * Behavior:
 *   - Iterate through `orders` in ascending order.
 *   - As soon as an entry’s timestamp > the given `timestamp`, return that timestamp.
 *   - If no such entry is found, return `orders[0].timestamp` (wrap around).
 */
std::string OrderBook::getNextTime(std::string timestamp)
{
    std::string next_timestamp = "";

    // Search for the first order whose timestamp is strictly greater
    for (OrderBookEntry& e : orders) {
        if (e.timestamp > timestamp) {
            next_timestamp = e.timestamp;
            break;
        }
    }

    // If none found, wrap around to earliest timestamp
    if (next_timestamp.empty()) {
        next_timestamp = orders[0].timestamp;
    }

    return next_timestamp;
}

/**
 * insertOrder
 * Inserts a new OrderBookEntry into `orders` and re‐sorts by timestamp so all
 * time‐based queries remain correct.
 *
 * @param order  The OrderBookEntry to insert.
 *
 * Behavior:
 *   1. Pushes `order` to the back of `orders`.
 *   2. Sorts the entire `orders` vector by ascending timestamp.
 *   (This is O(n log n) each time, but suffices for moderate data sizes.)
 */
void OrderBook::insertOrder(OrderBookEntry& order)
{
    orders.push_back(order);

    // Re‐sort by timestamp using compareByTimestamp helper
    std::sort(orders.begin(), orders.end(),
        OrderBookEntry::compareByTimestamp);
}

/**
 * matchAsksToBids
 * Simulates order matching at a given timestamp for a single product.
 * Matches as many ask orders to bid orders as possible, generating Sale entries.
 *
 * @param product    The product to match (e.g., "ETH/USDT")
 * @param timestamp  The exact time at which to match (e.g., "2020/06/01 12:00:00")
 *
 * @return A vector<OrderBookEntry> of sales that were executed. Each sale’s
 *         `orderType` is either asksale (if user sold) or bidsale (if user bought),
 *         and `price` is taken from the ask price when matched.
 *
 * Behavior:
 *   1. Fetch all asks and bids for this product/timestamp.
 *   2. If either side is empty, print a debug message and return empty.
 *   3. Sort asks by ascending price (lowest ask first).
 *   4. Sort bids by descending price (highest bid first).
 *   5. For each ask in the asks list:
 *        a. For each bid in the bids list:
 *             - If bid.price >= ask.price, they can trade at ask.price.
 *             - Create a new OrderBookEntry `sale` with:
 *                   price = ask.price
 *                   amount = determined below
 *                   product = product
 *                   timestamp = timestamp
 *                   orderType = asksale or bidsale depending on who was “simuser”
 *             - Determine matched quantity:
 *                   • If bid.amount == ask.amount: both sides fully match.
 *                   • If bid.amount > ask.amount: ask fully matched, adjust bid.
 *                   • If bid.amount < ask.amount: bid fully matched, adjust ask.
 *             - Push the new `sale` entry to `sales` vector.
 *             - Break or continue as appropriate once one side’s quantity is exhausted.
 *   6. Return the list of all `sales` created.
 */
std::vector<OrderBookEntry> OrderBook::matchAsksToBids(
    std::string product,
    std::string timestamp)
{
    // 1) Fetch asks and bids for the given product/timestamp
    std::vector<OrderBookEntry> asks = getOrders(
        OrderBookType::ask, product, timestamp);
    std::vector<OrderBookEntry> bids = getOrders(
        OrderBookType::bid, product, timestamp);

    // 2) Initialize an empty vector to hold any sales
    std::vector<OrderBookEntry> sales;

    // 3) If no asks or no bids, print debug and return empty sales
    if (asks.empty() || bids.empty()) {
        std::cout << "OrderBook::matchAsksToBids no bids or asks\n";
        return sales;
    }

    // 4) Sort asks lowest‐price first, bids highest‐price first
    std::sort(asks.begin(), asks.end(), OrderBookEntry::compareByPriceAsc);
    std::sort(bids.begin(), bids.end(), OrderBookEntry::compareByPriceDesc);

    // DEBUG: Print summary of best/worst prices
    std::cout << "max ask " << asks.back().price << "\n";
    std::cout << "min ask " << asks.front().price << "\n";
    std::cout << "max bid " << bids.front().price << "\n";
    std::cout << "min bid " << bids.back().price << "\n";

    // 5) Attempt to match each ask with available bids
    for (auto& ask : asks) {
        for (auto& bid : bids) {
            // If bid price >= ask price, a match can occur at ask.price
            if (bid.price >= ask.price) {
                // Create a sale entry with price = ask.price
                OrderBookEntry sale{
                    ask.price,        // matched price
                    0.0,              // placeholder for matched amount
                    timestamp,        // timestamp of trade
                    product,          // product being traded
                    OrderBookType::asksale // default; may change below
                };

                // If the bid side belonged to "simuser", mark as bidsale
                if (bid.username == "simuser") {
                    sale.username  = "simuser";
                    sale.orderType = OrderBookType::bidsale;
                }
                // If the ask side belonged to "simuser", mark as asksale
                if (ask.username == "simuser") {
                    sale.username  = "simuser";
                    sale.orderType = OrderBookType::asksale;
                }

                // Determine how much can be matched
                if (bid.amount == ask.amount) {
                    // 5a) Exact match in quantity: both sides fully matched
                    sale.amount = ask.amount;   // matched quantity
                    sales.push_back(sale);      // record the sale
                    bid.amount = 0.0;           // bid is fully consumed
                    // Move on to next ask
                    break;
                }
                else if (bid.amount > ask.amount) {
                    // 5b) Bid has larger quantity than ask: ask fully filled
                    sale.amount = ask.amount;   // matched quantity (ask side)
                    sales.push_back(sale);
                    bid.amount -= ask.amount;   // reduce bid by the matched amount
                    // Ask is fully consumed; move to next ask
                    break;
                }
                else {
                    // 5c) Bid has smaller quantity than ask: bid fully filled, ask partially remains
                    if (bid.amount > 0.0) {
                        sale.amount = bid.amount;   // matched quantity (bid side)
                        sales.push_back(sale);
                        ask.amount -= bid.amount;   // reduce ask by matched amount
                        bid.amount = 0.0;           // bid fully consumed
                        // Continue matching remaining ask amount against next bid
                        continue;
                    }
                }
            }
        }
    }

    return sales;
}

/**
 * getTradesPerProduct
 * Counts how many orders exist for each distinct product across all `orders`.
 *
 * @return A map<string,int> mapping each product (e.g., "BTC/USDT") to its total order count.
 *
 * Behavior:
 *   - Iterate through every OrderBookEntry in `orders`.
 *   - Increment counts[e.product] by one for each entry.
 *   - Return the map of product → count.
 */
std::map<std::string, int> OrderBook::getTradesPerProduct()
{
    std::map<std::string, int> counts;
    for (auto& entry : orders) {
        counts[entry.product]++;
    }
    return counts;
}

/**
 * getMeanPriceData
 * Computes the average price per minute for the specified side/product.
 *
 * @param type      OrderBookType (ask or bid)
 * @param product   The product (e.g., "ETH/USDT")
 * @return A vector of (minute, averagePrice) pairs, where:
 *           - minute: string "HH:MM" extracted from entry.timestamp
 *           - averagePrice: average of all entry.price values in that minute, rounded to 6 decimals.
 *
 * Behavior:
 *   1. Build a map from "HH:MM" → vector<double> of prices:
 *        - For each OrderBookEntry in `orders`:
 *            • If entry.orderType == type and entry.product == product
 *            • Extract `minute = entry.timestamp.substr(11, 5)` (characters 11–15, "HH:MM")
 *            • Append entry.price to pricesByMinute[minute].
 *
 *   2. For each (minute, priceList) in pricesByMinute:
 *        - Compute `avg = ∑priceList / priceList.size()`
 *        - Round `avg` to 6 decimal places: `avg = round(avg * 1e6) / 1e6`
 *        - Push `(minute, avg)` into the result vector.
 *
 *   3. Returns the resulting vector of (minute, avg) pairs. (Normalization for charting happens in TextPlotter.)
 */
std::vector<std::pair<std::string, double>> OrderBook::getMeanPriceData(
    OrderBookType type,
    const std::string& product)
{
    // 1) Group prices by "HH:MM"
    std::map<std::string, std::vector<double>> pricesByMinute;
    for (const auto& entry : orders) {
        if (entry.orderType == type && entry.product == product) {
            // Extract substring "HH:MM" from "YYYY/MM/DD HH:MM:SS.ffffff"
            std::string minute = entry.timestamp.substr(11, 5);
            pricesByMinute[minute].push_back(entry.price);
        }
    }

    // 2) Compute average price per minute
    std::vector<std::pair<std::string, double>> result;
    for (auto& [minute, priceList] : pricesByMinute) {
        if (priceList.empty()) {
            continue;  // Should not happen, but guard anyway
        }
        double sum = 0.0;
        for (double p : priceList) {
            sum += p;
        }
        double avg = sum / priceList.size();
        // Round to 6 decimal places for display clarity
        avg = std::round(avg * 1e6) / 1e6;
        result.emplace_back(minute, avg);
    }

    // 3) Return the (minute, avg) vector. Normalization to bar length is done in TextPlotter.
    return result;
}
