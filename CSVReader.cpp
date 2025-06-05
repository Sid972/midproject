#include "CSVReader.h"
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

/**
 * CSVReader:
 *   Utility class for reading order‐book entries from CSV files
 *   and tokenizing individual lines into fields. Provides methods
 *   to convert those tokens into OrderBookEntry objects, as well
 *   as a helper to gather all unique timestamps across multiple CSVs.
 */

/**
 * Default constructor
 * Does not perform any initialization, as all methods are static.
 */
CSVReader::CSVReader()
{
    // No state to initialize; methods below operate purely on inputs.
}

/**
 * readCSV
 * Reads a CSV file of order‐book entries, line by line, and converts each
 * valid line into an OrderBookEntry. If any line is malformed, it is skipped.
 *
 * @param csvFilename  Path to the CSV file (e.g., "20200317.csv").
 * @return A vector of OrderBookEntry objects parsed from the file.
 *
 * Behavior:
 *   - Opens the file in ifstream.
 *   - While not at end of file, reads each line as a std::string.
 *   - Tries to tokenize the line on commas and convert tokens into an OrderBookEntry.
 *   - If tokenization or conversion fails (e.g., wrong number of tokens or invalid number),
 *     catches the exception, logs a “bad data” message, and continues to next line.
 *   - At end, prints the total number of successfully read entries and returns them.
 */
std::vector<OrderBookEntry> CSVReader::readCSV(std::string csvFilename)
{
    std::vector<OrderBookEntry> entries;       // Will hold all successfully parsed entries

    std::ifstream csvFile{csvFilename};        // Attempt to open the CSV
    std::string line;

    if (csvFile.is_open()) {
        // Read each line until EOF
        while (std::getline(csvFile, line)) {
            try {
                // 1) Tokenize the line into a vector<string>
                std::vector<std::string> tokens = tokenise(line, ',');
                // 2) Convert the tokens to an OrderBookEntry
                OrderBookEntry obe = stringsToOBE(tokens);
                // 3) Add to the results
                entries.push_back(obe);
            }
            catch (const std::exception& e) {
                // Malformed line: skip, but print a warning
                std::cout << "CSVReader::readCSV bad data\n";
            }
        }
    } else {
        // If the file did not open, you could print an error here.
        std::cout << "CSVReader::readCSV could not open file: " << csvFilename << "\n";
    }

    // After reading all lines, log how many entries were parsed
    std::cout << "CSVReader::readCSV read " << entries.size() << " entries\n";
    return entries;
}

/**
 * tokenise
 * Splits a single CSV line into individual fields based on a given separator.
 *
 * @param csvLine    The raw line string from the CSV (e.g., "2020/03/17 12:00:00,ETH/USDT,ask,200,0.5")
 * @param separator  The character to split on (normally ',' for CSV).
 * @return A vector of strings, each representing one field. If the line is empty
 *         or consists only of separators, returns an empty vector.
 *
 * Behavior:
 *   - Finds the first non‐separator character as `start`.
 *   - Repeatedly finds the next separator position as `end`.
 *   - Extracts the substring between `start` and `end - 1`.
 *   - Pushes that substring into `tokens`.
 *   - Advances `start = end + 1` and repeats until no more fields remain.
 *   - Stops if `start` is at or beyond the line length, or if `start == end`.
 */
std::vector<std::string> CSVReader::tokenise(std::string csvLine, char separator)
{
    std::vector<std::string> tokens;
    signed int start, end;
    std::string token;

    // Find first character that is not the separator
    start = csvLine.find_first_not_of(separator, 0);

    do {
        // Find next occurrence of separator
        end = csvLine.find_first_of(separator, start);
        // If start is past the end of the string, or start == end (empty token), break
        if (start == static_cast<int>(csvLine.length()) || start == end) {
            break;
        }
        // If we found a separator at index `end`, extract substring [start .. end-1]
        if (end >= 0) {
            token = csvLine.substr(start, end - start);
        }
        // If no more separators (end == npos), extract from start to end of string
        else {
            token = csvLine.substr(start, csvLine.length() - start);
        }
        tokens.push_back(token);
        // Advance start to character after this separator
        start = end + 1;
    } while (end > 0);

    return tokens;
}

/**
 * stringsToOBE (overload #1)
 * Converts a vector of 5 tokens (strings) into an OrderBookEntry object:
 *   tokens[0] = timestamp (string)
 *   tokens[1] = product   (string, e.g., "ETH/USDT")
 *   tokens[2] = side      (string, either "ask" or "bid")
 *   tokens[3] = price     (string representing a double)
 *   tokens[4] = amount    (string representing a double)
 *
 * @param tokens  A vector<string> of length exactly 5.
 * @return An OrderBookEntry constructed from these tokens.
 *
 * Behavior:
 *   - If tokens.size() != 5, prints "Bad line" and throws an exception.
 *   - Attempts to convert price and amount tokens to doubles using std::stod.
 *   - If conversion fails, prints a “Bad float!” message for each offending token and rethrows.
 *   - Uses OrderBookEntry::stringToOrderBookType(tokens[2]) to convert "ask"/"bid" to enum.
 *   - Returns a new OrderBookEntry with (price, amount, timestamp, product, orderType).
 */
OrderBookEntry CSVReader::stringsToOBE(std::vector<std::string> tokens)
{
    double price, amount;

    // Ensure exactly 5 tokens
    if (tokens.size() != 5) {
        std::cout << "Bad line\n";
        throw std::exception{};
    }
    // Convert tokens[3] and tokens[4] to doubles
    try {
        price  = std::stod(tokens[3]);
        amount = std::stod(tokens[4]);
    }
    catch (const std::exception& e) {
        // If conversion fails, log which fields could not be parsed
        std::cout << "CSVReader::stringsToOBE Bad float! " << tokens[3] << "\n";
        std::cout << "CSVReader::stringsToOBE Bad float! " << tokens[4] << "\n";
        throw;  // Propagate exception to caller
    }

    // Construct a new OrderBookEntry:
    //   - price      = parsed double
    //   - amount     = parsed double
    //   - timestamp  = tokens[0]
    //   - product    = tokens[1]
    //   - orderType  = enum converted from tokens[2]
    OrderBookEntry obe{
        price,
        amount,
        tokens[0],                                      // timestamp
        tokens[1],                                      // product
        OrderBookEntry::stringToOrderBookType(tokens[2]) // convert "ask"/"bid"
    };

    return obe;
}

/**
 * stringsToOBE (overload #2)
 * Converts individual string fields into an OrderBookEntry:
 *   priceString  = string representing a double (e.g., "200.5")
 *   amountString = string representing a double (e.g., "0.75")
 *   timestamp    = full timestamp string (e.g., "2020/06/01 12:00:00.000000")
 *   product      = currency pair (e.g., "ETH/USDT")
 *   orderType    = OrderBookType enum (ask or bid)
 *
 * @param priceString   Price as string
 * @param amountString  Amount as string
 * @param timestamp     Timestamp as string
 * @param product       Product as string
 * @param orderType     Already‐parsed enum for ask/bid
 * @return A new OrderBookEntry constructed from these fields.
 *
 * Behavior:
 *   - Attempts to convert priceString and amountString to double using std::stod.
 *   - If conversion fails, prints a “Bad float!” message and rethrows.
 *   - Constructs and returns an OrderBookEntry with (price, amount, timestamp, product, orderType).
 */
OrderBookEntry CSVReader::stringsToOBE(
    std::string priceString,
    std::string amountString,
    std::string timestamp,
    std::string product,
    OrderBookType orderType)
{
    double price, amount;
    try {
        price  = std::stod(priceString);
        amount = std::stod(amountString);
    }
    catch (const std::exception& e) {
        std::cout << "CSVReader::stringsToOBE Bad float! " << priceString << "\n";
        std::cout << "CSVReader::stringsToOBE Bad float! " << amountString << "\n";
        throw;  // Propagate to caller
    }
    // Construct and return the OrderBookEntry
    OrderBookEntry obe{
        price,
        amount,
        timestamp,
        product,
        orderType
    };
    return obe;
}

/**
 * getAllTimestamps
 * Gathers every unique timestamp (string) from a predefined list of CSV files,
 * then returns them as a sorted vector. Used to build candlestick data over time.
 *
 * @return A sorted std::vector<std::string> containing every distinct timestamp
 *         found in all listed CSV files.
 *
 * Behavior:
 *   - Defines a hardcoded list of CSV file names (e.g., "20200317.csv", "20200601.csv").
 *   - For each filename:
 *       • Calls readCSV(filename) to load all OrderBookEntry objects.
 *       • Inserts each entry.timestamp into a std::set<std::string> to ensure uniqueness.
 *   - After processing all files, converts the set (automatically sorted) into a vector and returns it.
 *
 * Note:
 *   - This method assumes those CSV files are located in the working directory.
 *   - If you add more CSV files, you must update the `files` vector accordingly.
 */
std::vector<std::string> CSVReader::getAllTimestamps() {
    std::set<std::string> uniq;  // Will store unique timestamps in sorted order

    // List all CSV files from which to collect timestamps
    const std::vector<std::string> files = {
        "20200317.csv",
        "20200601.csv"
    };

    // For each CSV file, read all entries and insert each timestamp into the set
    for (auto& filename : files) {
        std::vector<OrderBookEntry> entries = CSVReader().readCSV(filename);
        for (auto& e : entries) {
            uniq.insert(e.timestamp);
        }
    }

    // Convert the set into a sorted vector and return
    return std::vector<std::string>(uniq.begin(), uniq.end());
}
