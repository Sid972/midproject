#include "CurrencySelector.h"
#include <QListWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>

CurrencySelector::CurrencySelector(const QStringList& products,
                                   QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select trading pairs");
    resize(250, 300);

    // ---- the check-list ----
    m_list = new QListWidget(this);
    m_list->addItems(products);
    for (int i = 0; i < m_list->count(); ++i) {
        auto *it = m_list->item(i);
        it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        it->setCheckState(Qt::Unchecked);
    }

    // ---- OK / Cancel buttons ----
    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // ---- lay out widgets ----
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(m_list);
    lay->addWidget(buttons);
}

QStringList CurrencySelector::selectedProducts() const
{
    QStringList out;
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->checkState() == Qt::Checked)
            out << m_list->item(i)->text();
    return out;
}
