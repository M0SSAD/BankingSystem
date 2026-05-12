#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include <cstdint>
#include "types.h"

struct TransactionDisplay {
	uint64_t transactionId;
	uint64_t srcId;
	uint64_t destId;
	int64_t amount;
	TransactionStatus status;
	QString timestamp;
};

class AuditLogModel : public QAbstractTableModel {
	Q_OBJECT
public:
	enum Columns {
		ColTxId,
		ColFrom,
		ColTo,
		ColAmount,
		ColStatus,
		ColTime,
		ColCount
	};

	explicit AuditLogModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = {}) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index,
	              int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
	                    int role = Qt::DisplayRole) const override;

	void setTransactions(const QVector<TransactionDisplay> &transactions);
	void clear();

private:
	QVector<TransactionDisplay> m_transactions;
};

Q_DECLARE_METATYPE(TransactionDisplay)
Q_DECLARE_METATYPE(QVector<TransactionDisplay>)
