#include "MerkelMain.h"
#include "CSVReader.h"
#include "OrderBookEntry.h"
#include "TextPlotter.h"
#include "Candlestick.h"
#include <iostream>
#include <cstdlib>

MerkelMain::MerkelMain(OrderBook& book,
                       Wallet&    wal,
                       const std::vector<std::string>& prods,
                       QObject* parent)
  : QObject(parent)
  , orderBook(book)
  , wallet(wal)
  , products(prods)
{
    // set the starting time
    currentTime = orderBook.getEarliestTime();
}

void MerkelMain::printMenu()
{
    std::cout
      << "1: Print help\n"
      << "2: Print exchange stats\n"
      << "3: Make an offer\n"
      << "4: Make a bid\n"
      << "5: Print wallet\n"
      << "6: Continue\n"
      << "7: Print candlestick chart\n"
      << "8: Print volume chart\n"
    << "9: Print average price chart\n"
    << "10: Print number of trades per product\n"
      << "0: Quit\n"
      << "Enter option: ";
}

int MerkelMain::getUserOption()
{
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    } catch (...) {
        return -1;
    }
}

void MerkelMain::processUserOption(int choice)
{
    switch (choice)
    {
      case 1: printHelp();             break;
      case 2: printMarketStats();      break;
      case 3: enterAsk();              break;
      case 4: enterBid();              break;
      case 5: printWallet();           break;
      case 6: gotoNextTimeframe();     break;
      case 7: printCandlestickChart(); break;
      case 8: printVolumeChart();      break;
        case 9: printMeanPriceChart(); break;
        case 10: printTradesPerProduct(); break;
      case 0: std::exit(0);            break;
      default:
        std::cout << "Invalid choice, please type 0–8\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Below here, all your existing helpers from before—unchanged.
// ─────────────────────────────────────────────────────────────────────────────

void MerkelMain::printHelp()
{
    std::cout << "Help - your aim is to make money. Analyse the market and make bids and offers.\n";
}

void MerkelMain::printMarketStats()
{
    for (auto const& p : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << p << "\n";
        auto asks = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        std::cout << "Asks seen: " << asks.size() << "\n";
        std::cout << "Max ask: " << OrderBook::getHighPrice(asks) << "\n";
        std::cout << "Min ask: " << OrderBook::getLowPrice(asks) << "\n";
    }
}

void MerkelMain::enterAsk()
{
    std::cout << "Make an ask - enter product,price,amount (e.g. ETH/BTC,200,0.5):\n";
    std::string line;
    std::getline(std::cin, line);
    auto tokens = CSVReader::tokenise(line, ',');
    if (tokens.size() != 3)
    {
        std::cout << "Bad input: " << line << "\n";
        return;
    }
    try {
        auto obe = CSVReader::stringsToOBE(
            tokens[1], tokens[2], currentTime, tokens[0], OrderBookType::ask);
        obe.username = "simuser";
        if (wallet.canFulfillOrder(obe)) {
            orderBook.insertOrder(obe);
            std::cout << "Ask placed.\n";
        } else {
            std::cout << "Insufficient funds.\n";
        }
    } catch (...) {
        std::cout << "Error parsing input.\n";
    }
}

void MerkelMain::enterBid()
{
    std::cout << "Make a bid - enter product,price,amount (e.g. ETH/BTC,200,0.5):\n";
    std::string line;
    std::getline(std::cin, line);
    auto tokens = CSVReader::tokenise(line, ',');
    if (tokens.size() != 3)
    {
        std::cout << "Bad input: " << line << "\n";
        return;
    }
    try {
        auto obe = CSVReader::stringsToOBE(
            tokens[1], tokens[2], currentTime, tokens[0], OrderBookType::bid);
        obe.username = "simuser";
        if (wallet.canFulfillOrder(obe)) {
            orderBook.insertOrder(obe);
            std::cout << "Bid placed.\n";
        } else {
            std::cout << "Insufficient funds.\n";
        }
    } catch (...) {
        std::cout << "Error parsing input.\n";
    }
}

void MerkelMain::printWallet()
{
    std::cout << wallet.toString() << "\n";
}

void MerkelMain::gotoNextTimeframe()
{
    std::cout << "Going to next time frame...\n";
    for (auto const& p : orderBook.getKnownProducts())
    {
        auto sales = orderBook.matchAsksToBids(p, currentTime);
        for (auto& sale : sales)
        {
            std::cout << "Sale " << p
                      << " price: " << sale.price
                      << " amount: " << sale.amount << "\n";
            if (sale.username == "simuser")
                wallet.processSale(sale);
        }
    }
    currentTime = orderBook.getNextTime(currentTime);
}

void MerkelMain::printCandlestickChart() {
    std::cout << "Enter product for candlestick (e.g. ETH/USDT): ";
    std::string prod;
    std::getline(std::cin, prod);
    if (prod.empty()) std::getline(std::cin, prod);

    auto candles = orderBook.getCandlestickData(OrderBookType::ask, prod);
    const size_t MAX_CANDLES = 50;
        if (candles.size() > MAX_CANDLES) {
                // keep only the final MAX_CANDLES entries
                candles = std::vector<Candlestick>(
                    candles.end() - MAX_CANDLES,
                    candles.end()
                );
           }

    TextPlotter::drawCandlesticks(candles);
}
void MerkelMain::printVolumeChart()
{
    std::cout << "Enter product for volume chart (e.g. ETH/USDT): ";
    std::string prod;
    std::getline(std::cin, prod);
    if (prod.empty()) std::getline(std::cin, prod);

    auto vol = orderBook.getVolumeData(OrderBookType::ask, prod);
    TextPlotter::drawVolumeChart(vol);
}
void MerkelMain::printMeanPriceChart()
{
    // Show available products
    std::cout << "Available products:\n";
    for (auto const& p : orderBook.getKnownProducts())
        std::cout << "  - " << p << "\n";

    // Ask for product
    std::cout << "Enter product (e.g. ETH/USDT): ";
    std::string prod;
    std::getline(std::cin, prod);
    if (prod.empty()) std::getline(std::cin, prod);

    // Ask for side
    std::cout << "Plot mean price for (1) ask  or  (2) bid?  Enter 1 or 2: ";
    std::string choice;
    std::getline(std::cin, choice);
    OrderBookType side = (choice == "1" ? OrderBookType::ask : OrderBookType::bid);

    // Fetch and draw
    auto data = orderBook.getMeanPriceData(side, prod);
    if (data.empty()) {
        std::cout << "No mean price data for \"" << prod << "\" on that side.\n";
        return;
    }
    TextPlotter::drawMeanPriceChart(data);
}


void MerkelMain::printTradesPerProduct()
{
    auto counts = orderBook.getTradesPerProduct();
    std::cout << "Total trades per product:\n";
    for (auto& [product, count] : counts) {
        std::cout << product << ": " << count << " orders\n";
    }
}
