#include "AuditLogModel.h"
#include <QBrush>
#include <QFont>
#include <QLocale>

AuditLogModel::AuditLogModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int AuditLogModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return m_transactions.size();
}

int AuditLogModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return ColCount;
}

static QString txStatusToString(TransactionStatus status) {
	switch (status) {
	case TransactionStatus::SUCCESS:
		return QStringLiteral("SUCCESS");
	case TransactionStatus::FAILED_INSUFFICIENT_FUNDS:
		return QStringLiteral("INSUFFICIENT FUNDS");
	case TransactionStatus::FAILED_ACCOUNT_NOT_FOUND:
		return QStringLiteral("ACCOUNT NOT FOUND");
	case TransactionStatus::FAILED_ACCOUNT_FROZEN:
		return QStringLiteral("ACCOUNT FROZEN");
	case TransactionStatus::FAILED_INVALID_STATE:
		return QStringLiteral("INVALID STATE");
	case TransactionStatus::FAILED_INVALID_AMOUNT:
		return QStringLiteral("INVALID AMOUNT");
	case TransactionStatus::FAILED_ACCOUNT_PENDING:
		return QStringLiteral("ACCOUNT PENDING");
	case TransactionStatus::FAILED_ACCOUNT_CLOSED:
		return QStringLiteral("ACCOUNT CLOSED");
	case TransactionStatus::FAILED_ACCOUNT_NOT_ACTIVE:
		return QStringLiteral("ACCOUNT NOT ACTIVE");
	case TransactionStatus::FAILED_SAME_ACCOUNT:
		return QStringLiteral("SAME ACCOUNT");
	}
	return QStringLiteral("UNKNOWN");
}

QVariant AuditLogModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid() || index.row() >= m_transactions.size())
		return {};

	const auto &tx = m_transactions[index.row()];

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColTxId:
			return QString::number(tx.transactionId);
		case ColFrom:
			return tx.srcId == 0 ? QStringLiteral("---")
			                     : QString::number(tx.srcId);
		case ColTo:
			return tx.destId == 0 ? QStringLiteral("---")
			                      : QString::number(tx.destId);
		case ColAmount: {
			static const QLocale loc(QLocale::English);
			return loc.toString(static_cast<qlonglong>(tx.amount));
		}
		case ColStatus:
			return txStatusToString(tx.status);
		case ColTime:
			return tx.timestamp;
		}
	}

	if (role == Qt::ForegroundRole) {
		if (index.column() == ColStatus) {
			if (tx.status == TransactionStatus::SUCCESS)
				return QBrush(QColor("#059669"));
			return QBrush(QColor("#DC2626"));
		}
		if (index.column() == ColAmount) {
			if (tx.srcId == 0 && tx.destId != 0)
				return QBrush(QColor("#059669"));
			if (tx.srcId != 0 && tx.destId == 0)
				return QBrush(QColor("#D97706"));
		}
	}

	if (role == Qt::TextAlignmentRole) {
		if (index.column() == ColAmount)
			return int(Qt::AlignRight | Qt::AlignVCenter);
		return int(Qt::AlignCenter);
	}

	if (role == Qt::FontRole) {
		QFont font("Monospace");
		font.setStyleHint(QFont::Monospace);
		return font;
	}

	return {};
}

QVariant AuditLogModel::headerData(int section,
                                   Qt::Orientation orientation,
                                   int role) const {
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return {};
	switch (section) {
	case ColTxId:
		return QStringLiteral("Tx ID");
	case ColFrom:
		return QStringLiteral("From");
	case ColTo:
		return QStringLiteral("To");
	case ColAmount:
		return QStringLiteral("Amount (EGP)");
	case ColStatus:
		return QStringLiteral("Status");
	case ColTime:
		return QStringLiteral("Time");
	}
	return {};
}

void AuditLogModel::setTransactions(
    const QVector<TransactionDisplay> &transactions) {
	beginResetModel();
	m_transactions = transactions;
	endResetModel();
}

void AuditLogModel::clear() {
	beginResetModel();
	m_transactions.clear();
	endResetModel();
}
