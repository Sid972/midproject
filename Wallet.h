#pragma once

#include <string>
#include <map>
#include "OrderBookEntry.h"
#include <iostream>
/**
 * Wallet: keeps track of multiple currency balances.
 *   - insertCurrency(type, amount) to add funds
 *   - removeCurrency(type, amount) to deduct
 *   - containsCurrency(type, amount) to check sufficiency
 *   - canFulfillOrder(order) to see if wallet can pay that ask/bid
 *   - processSale(order) to update balances when a sale completes
 *   - toString() to view current holdings
 */
class Wallet 
{
    public:
        Wallet();
        /** insert currency to the wallet */
        void insertCurrency(std::string type, double amount);
        /** remove currency from the wallet */
        bool removeCurrency(std::string type, double amount);
        
        /** check if the wallet contains this much currency or more */
        bool containsCurrency(std::string type, double amount);
        /** checks if the wallet can cope with this ask or bid.*/
        bool canFulfillOrder(OrderBookEntry order);
        /** update the contents of the wallet
         * assumes the order was made by the owner of the wallet
        */
        void processSale(OrderBookEntry& sale);


        /** generate a string representation of the wallet */
        std::string toString();
        friend std::ostream& operator<<(std::ostream& os, Wallet& wallet);

        
    private:
        std::map<std::string,double> currencies;

};



	

