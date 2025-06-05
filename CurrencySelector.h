#pragma once
#include <QDialog>
#include <QStringList>

class QListWidget;

/* Small modal dialog that lets the user tick the pairs
 * they want to trade.  Call selectedProducts() after exec(). */
class CurrencySelector : public QDialog
{
    Q_OBJECT
public:
    explicit CurrencySelector(const QStringList& products,
                              QWidget* parent = nullptr);

    QStringList selectedProducts() const;

private:
    QListWidget* m_list;
};
