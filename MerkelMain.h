#pragma once

#include <QObject>
#include <vector>
#include <string>

#include "OrderBook.h"    // full definition now available
#include "Wallet.h"       // full definition now available
/**
 * MerkelMain: The main CLI controller for your text‐based exchange simulation.
 *   - Offers a menu of options (help, stats, make ask, make bid, wallet, next timeframe,
 *     candlestick chart, volume chart, mean price chart, trade‐counts, quit).
 *   - Holds references to the OrderBook, Wallet, and selected products.
 */
class MerkelMain : public QObject {
    Q_OBJECT

public:
    // match this signature exactly in your .cpp
    explicit MerkelMain(OrderBook& book,
                        Wallet& wallet,
                        const std::vector<std::string>& products,
                        QObject* parent = nullptr);
    void printMenu();
    int  getUserOption();
    void processUserOption(int userOption);
    // your public API
    //void init();

private:
    // the CLI/GUI hybrid helpers you call from init()


    // the rest of your old text‐mode methods:
    void printHelp();
    void printMarketStats();
    void enterAsk();
    void enterBid();
    void printWallet();
    void gotoNextTimeframe();

    // slots for your Qt buttons/menus, if you wired them up:
    private slots:
        void printCandlestickChart();// TASK 1: Candlestick
    void printVolumeChart();// TASK 3a: Volume chart
    void printMeanPriceChart(); // TASK 2: Mean price chart (per minute)
    void printTradesPerProduct();// TASK 4: Print number of trades per product

private:
    OrderBook&              orderBook;
    Wallet&                 wallet;
    std::vector<std::string> products;
    std::string             currentTime;
};
