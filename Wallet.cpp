#include "Wallet.h"
#include <iostream>
#include "CSVReader.h"

/**
 * Wallet:
 * Manages balances of different currencies for a user,
 * allowing deposits, withdrawals, and checking sufficiency
 * to fulfill orders. Also processes order results (sales).
 */

/**
 * Default constructor:
 * Initializes an empty wallet with no currency balances.
 */
Wallet::Wallet()
{
    // Initially, the `currencies` map is empty. No further setup needed.
}

/**
 * insertCurrency
 * Adds `amount` of a given `type` currency into the wallet.
 *
 * @param type   The currency ticker (e.g., "BTC", "ETH", "USDT").
 * @param amount The amount to insert (must be non-negative).
 *
 * @throws std::exception if `amount` is negative.
 *
 * Behavior:
 *   - If `type` does not exist in the `currencies` map, it initializes its balance to 0.
 *   - Adds `amount` to the existing balance for `type`.
 *   - If `amount` < 0, an exception is thrown.
 */
void Wallet::insertCurrency(std::string type, double amount)
{
    if (amount < 0) {
        // Negative deposit is not allowed.
        throw std::exception{};
    }

    double balance = 0.0;
    // Check if this currency already exists in the map.
    if (currencies.count(type) > 0) {
        balance = currencies[type];  // existing balance
    }
    // Increase balance by the deposit amount.
    balance += amount;
    // Update (or create) the entry in the map.
    currencies[type] = balance;
}

/**
 * removeCurrency
 * Attempts to subtract `amount` of a given `type` currency from the wallet.
 *
 * @param type   The currency ticker to remove from (e.g., "BTC", "USDT").
 * @param amount The amount to remove (must be non-negative).
 *
 * @return true if removal succeeded (wallet had enough balance), false otherwise.
 *
 * Behavior:
 *   - If `amount` is negative, returns false (invalid).
 *   - If `type` is not present in the wallet, returns false.
 *   - If the wallet has at least `amount` of `type`, subtract it and return true.
 *   - Otherwise (insufficient funds), return false.
 */
bool Wallet::removeCurrency(std::string type, double amount)
{
    if (amount < 0) {
        // Cannot remove a negative amount.
        return false;
    }

    // If currency is not in the wallet at all, cannot remove.
    if (currencies.count(type) == 0) {
        return false;
    }

    // If there is enough balance, subtract and return true.
    if (containsCurrency(type, amount)) {
        currencies[type] -= amount;
        return true;
    }

    // Otherwise, insufficient funds.
    return false;
}

/**
 * containsCurrency
 * Checks if the wallet has at least `amount` of `type` currency.
 *
 * @param type   The currency ticker to check (e.g., "BTC", "ETH").
 * @param amount The required amount.
 *
 * @return true if wallet[type] >= amount, false otherwise.
 */
bool Wallet::containsCurrency(std::string type, double amount)
{
    // If currency is not in the wallet map, balance is effectively zero.
    if (currencies.count(type) == 0) {
        return false;
    }
    // Otherwise, compare stored balance to requested amount.
    return currencies[type] >= amount;
}

/**
 * toString
 * Generates a multi-line string representation of the entire wallet.
 *
 * Format:
 *   <CURRENCY> : <AMOUNT>\n
 *   <CURRENCY> : <AMOUNT>\n
 *   ...
 *
 * Example:
 *   "BTC : 0.500000\nETH : 10.000000\nUSDT : 250.000000\n"
 *
 * @return A std::string containing each currency and its balance.
 */
std::string Wallet::toString()
{
    std::string s;
    for (auto const& pair : currencies) {
        const std::string& currency = pair.first;
        double amount = pair.second;
        // Append "currency : amount\n"
        s += currency + " : " + std::to_string(amount) + "\n";
    }
    return s;
}

/**
 * canFulfillOrder
 * Determines if the wallet has sufficient funds to place a given order.
 *
 * Orders in this exchange are always of the form "BASE/QUOTE", e.g. "ETH/USDT".
 *   - For an ask (sell), the user must have at least `amount` of BASE currency.
 *   - For a bid (buy), the user must have at least `amount * price` of QUOTE currency.
 *
 * @param order The OrderBookEntry to check against the wallet’s balances.
 *
 * @return true if the wallet can cover the order, false if not.
 *
 * Behavior:
 *   - Tokenizes the product string `BASE/QUOTE` into ["BASE","QUOTE"].
 *   - If orderType == ask:
 *       Need `amount` units of BASE.
 *   - If orderType == bid:
 *       Need `amount * price` units of QUOTE.
 */
bool Wallet::canFulfillOrder(OrderBookEntry order)
{
    // Split the product "BASE/QUOTE" into two tokens.
    std::vector<std::string> currs = CSVReader::tokenise(order.product, '/');

    // If this is a sell order (ask), check if we have enough BASE (currs[0]).
    if (order.orderType == OrderBookType::ask) {
        double amountNeeded = order.amount;       // amount of BASE to sell
        std::string baseCurrency = currs[0];
        std::cout << "Wallet::canFulfillOrder " << baseCurrency
                  << " : " << amountNeeded << std::endl;
        return containsCurrency(baseCurrency, amountNeeded);
    }

    // If this is a buy order (bid), check if we have enough QUOTE (currs[1]).
    if (order.orderType == OrderBookType::bid) {
        double quoteNeeded = order.amount * order.price;  // amount of QUOTE to pay
        std::string quoteCurrency = currs[1];
        std::cout << "Wallet::canFulfillOrder " << quoteCurrency
                  << " : " << quoteNeeded << std::endl;
        return containsCurrency(quoteCurrency, quoteNeeded);
    }

    // For any other order type, we cannot fulfill it.
    return false;
}

/**
 * processSale
 * Updates wallet balances after an executed sale (match between ask & bid).
 *
 * There are two sale types:
 *   - asksale: The user’s ask order was matched. They sold BASE, so:
 *         wallet[QUOTE] += sale.amount * sale.price;
 *         wallet[BASE]  -= sale.amount;
 *
 *   - bidsale: The user’s bid order was matched. They bought BASE, so:
 *         wallet[BASE]  += sale.amount;
 *         wallet[QUOTE] -= sale.amount * sale.price;
 *
 * @param sale The OrderBookEntry representing a sale. Its orderType is asksale or bidsale.
 */
void Wallet::processSale(OrderBookEntry& sale)
{
    // Split "BASE/QUOTE" into ["BASE","QUOTE"].
    std::vector<std::string> currs = CSVReader::tokenise(sale.product, '/');

    // If this sale is from an ask (user sold BASE):
    if (sale.orderType == OrderBookType::asksale) {
        double baseSold    = sale.amount;                // amount of BASE sold
        std::string base   = currs[0];                   // e.g., "ETH"
        double quoteGained = sale.amount * sale.price;   // USD received
        std::string quote  = currs[1];                   // e.g., "USDT"

        // Increase QUOTE balance, decrease BASE balance.
        currencies[quote] += quoteGained;
        currencies[base]  -= baseSold;
    }

    // If this sale is from a bid (user bought BASE):
    if (sale.orderType == OrderBookType::bidsale) {
        double baseGained  = sale.amount;                // amount of BASE bought
        std::string base   = currs[0];                   // e.g., "ETH"
        double quoteSpent  = sale.amount * sale.price;   // USD spent
        std::string quote  = currs[1];                   // e.g., "USDT"

        // Increase BASE balance, decrease QUOTE balance.
        currencies[base]  += baseGained;
        currencies[quote] -= quoteSpent;
    }
}

/**
 * operator<< overload
 * Allows printing the wallet directly via std::cout << wallet;
 * Simply forwards to wallet.toString().
 */
std::ostream& operator<<(std::ostream& os, Wallet& wallet)
{
    os << wallet.toString();
    return os;
}
