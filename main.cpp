#include <QApplication>
#include "CurrencySelector.h"
#include "OrderBook.h"
#include "Wallet.h"
#include "MerkelMain.h"
#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // ── 1) Load your data ───────────────────────
    OrderBook orderBook("20200317.csv", "20200601.csv");
    Wallet    wallet;
    wallet.insertCurrency("BTC", 10);

    // ── 2) Show the currency picker ────────────
    QStringList list;
    for (auto& p : orderBook.getKnownProducts())
        list << QString::fromStdString(p);

    CurrencySelector dlg(list);
    if (dlg.exec() != QDialog::Accepted)
        return 0;

    std::vector<std::string> chosen;
    for (auto& q : dlg.selectedProducts())
        chosen.push_back(q.toStdString());
    if (chosen.empty())
        return 0;

    // ── 3) Hand off to your CLI object ─────────
    MerkelMain cli(orderBook, wallet, chosen);

    // ── 4) Run the old CLI loop right here ─────
    while (true) {
        cli.printMenu();
        int choice = cli.getUserOption();
        if (choice == 0) break;           // 0 = Quit
        cli.processUserOption(choice);
    }

    return 0;
}
