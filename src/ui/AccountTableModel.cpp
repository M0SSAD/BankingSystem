#include "AccountTableModel.h"
#include <QBrush>
#include <QFont>
#include <QLocale>

AccountTableModel::AccountTableModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int AccountTableModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return m_accounts.size();
}

int AccountTableModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return ColCount;
}

static QString formatBalance(int64_t value) {
	static const QLocale loc(QLocale::English);
	return loc.toString(static_cast<qlonglong>(value));
}

QVariant AccountTableModel::data(const QModelIndex &index,
                                 int role) const {
	if (!index.isValid() || index.row() >= m_accounts.size())
		return {};

	const auto &acc = m_accounts[index.row()];

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColAccountId:
			return QString::number(acc.accountId);
		case ColCustomerId:
			return QString::number(acc.customerId);
		case ColName:
			return acc.customerName;
		case ColBalance:
			return formatBalance(acc.balance);
		case ColState:
			switch (acc.state) {
			case AccountState::PENDING:
				return QStringLiteral("PENDING");
			case AccountState::ACTIVE:
				return QStringLiteral("ACTIVE");
			case AccountState::FROZEN:
				return QStringLiteral("FROZEN");
			case AccountState::CLOSED:
				return QStringLiteral("CLOSED");
			}
		}
	}

	if (role == Qt::ForegroundRole) {
		if (index.column() == ColBalance) {
			return QBrush(acc.balance >= 0 ? QColor("#059669")
			                               : QColor("#DC2626"));
		}
		if (index.column() == ColState) {
			switch (acc.state) {
			case AccountState::ACTIVE:
				return QBrush(QColor("#059669"));
			case AccountState::FROZEN:
				return QBrush(QColor("#D97706"));
			case AccountState::CLOSED:
				return QBrush(QColor("#DC2626"));
			case AccountState::PENDING:
				return QBrush(QColor("#2563EB"));
			}
		}
	}

	if (role == Qt::TextAlignmentRole) {
		if (index.column() == ColBalance)
			return int(Qt::AlignRight | Qt::AlignVCenter);
		if (index.column() == ColAccountId ||
		    index.column() == ColCustomerId)
			return int(Qt::AlignCenter);
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	}

	if (role == Qt::FontRole) {
		if (index.column() == ColAccountId ||
		    index.column() == ColCustomerId ||
		    index.column() == ColBalance) {
			QFont font("Monospace");
			font.setStyleHint(QFont::Monospace);
			return font;
		}
	}

	return {};
}

QVariant AccountTableModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const {
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return {};
	switch (section) {
	case ColAccountId:
		return QStringLiteral("Account ID");
	case ColCustomerId:
		return QStringLiteral("Customer ID");
	case ColName:
		return QStringLiteral("Customer Name");
	case ColBalance:
		return QStringLiteral("Balance (EGP)");
	case ColState:
		return QStringLiteral("State");
	}
	return {};
}

void AccountTableModel::setAccounts(
    const QVector<AccountDisplay> &accounts) {
	beginResetModel();
	m_accounts = accounts;
	endResetModel();
}

void AccountTableModel::clear() {
	beginResetModel();
	m_accounts.clear();
	endResetModel();
}

CustomerTableModel::CustomerTableModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int CustomerTableModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return m_customers.size();
}

int CustomerTableModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return ColCount;
}

QVariant CustomerTableModel::data(const QModelIndex &index,
                                  int role) const {
	if (!index.isValid() || index.row() >= m_customers.size())
		return {};

	const auto &cust = m_customers[index.row()];

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColCustId:
			return QString::number(cust.customerId);
		case ColName:
			return cust.name;
		case ColNatId:
			return QString::number(cust.nationalId);
		case ColAccounts:
			return QString::number(cust.accountCount);
		}
	}

	if (role == Qt::TextAlignmentRole) {
		if (index.column() == ColCustId ||
		    index.column() == ColNatId ||
		    index.column() == ColAccounts)
			return int(Qt::AlignCenter);
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	}

	if (role == Qt::FontRole) {
		if (index.column() == ColCustId || index.column() == ColNatId) {
			QFont font("Monospace");
			font.setStyleHint(QFont::Monospace);
			return font;
		}
	}

	return {};
}

QVariant CustomerTableModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return {};
	switch (section) {
	case ColCustId:
		return QStringLiteral("Customer ID");
	case ColName:
		return QStringLiteral("Name");
	case ColNatId:
		return QStringLiteral("National ID");
	case ColAccounts:
		return QStringLiteral("Accounts");
	}
	return {};
}

void CustomerTableModel::setCustomers(
    const QVector<CustomerDisplay> &customers) {
	beginResetModel();
	m_customers = customers;
	endResetModel();
}

void CustomerTableModel::clear() {
	beginResetModel();
	m_customers.clear();
	endResetModel();
}
