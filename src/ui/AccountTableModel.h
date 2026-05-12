#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QVector>
#include <cstdint>
#include "types.h"

struct AccountDisplay {
	uint64_t accountId;
	uint64_t customerId;
	QString customerName;
	int64_t balance;
	AccountState state;
};

class AccountTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	enum Columns {
		ColAccountId,
		ColCustomerId,
		ColName,
		ColBalance,
		ColState,
		ColCount
	};

	explicit AccountTableModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = {}) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index,
	              int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
	                    int role = Qt::DisplayRole) const override;

	void setAccounts(const QVector<AccountDisplay> &accounts);
	void clear();

private:
	QVector<AccountDisplay> m_accounts;
};

struct CustomerDisplay {
	uint64_t customerId;
	QString name;
	uint64_t nationalId;
	int accountCount;
};

class CustomerTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	enum Columns { ColCustId, ColName, ColNatId, ColAccounts, ColCount };

	explicit CustomerTableModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = {}) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index,
	              int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
	                    int role = Qt::DisplayRole) const override;

	void setCustomers(const QVector<CustomerDisplay> &customers);
	void clear();

private:
	QVector<CustomerDisplay> m_customers;
};

Q_DECLARE_METATYPE(CustomerDisplay)
Q_DECLARE_METATYPE(QVector<CustomerDisplay>)
