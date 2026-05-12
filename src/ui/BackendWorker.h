#pragma once

#include <QObject>
#include <QPair>
#include <QString>
#include <QVector>
#include <cstdint>
#include "AccountTableModel.h"
#include "AuditLogModel.h"
#include "Bank.h"

class BackendWorker : public QObject {
	Q_OBJECT
public:
	explicit BackendWorker(QObject *parent = nullptr);

public slots:
	void onLookupCustomer(uint64_t nationalId);
	void onRegisterCustomer(const QString &name, uint64_t nationalId);
	void onOpenAccount(uint64_t customerId);
	void onDeposit(uint64_t accountId, int64_t amount);
	void onWithdraw(uint64_t accountId, int64_t amount);
	void onTransfer(uint64_t from, uint64_t to, int64_t amount);
	void onFreezeAccount(uint64_t accountId);
	void onUnfreezeAccount(uint64_t accountId);
	void onCloseAccount(uint64_t accountId);
	void onSearchCustomer(const QString &prefix);
	void onRequestAllAccounts();
	void onRequestAllCustomers();
	void onRequestAllTransactions();
	void onRequestCustomerAccounts(uint64_t customerId);
	void onRequestCustomerTransactions(uint64_t customerId);

signals:
	void customerLookupResult(bool found, uint64_t customerId,
	                          const QString &name,
	                          const QVector<uint64_t> &accountIds);
	void operationResult(const QString &message, bool success);
	void accountsReady(const QVector<AccountDisplay> &accounts);
	void customersReady(const QVector<CustomerDisplay> &customers);
	void transactionsReady(
	    const QVector<TransactionDisplay> &transactions);
	void searchResultsReady(
	    const QVector<QPair<uint64_t, QString>> &results);

private:
	Bank m_bank;

	static QString formatTimestamp(
	    const std::chrono::system_clock::time_point &tp);
	static QString txStatusMessage(TransactionStatus status);
	static QString acctOpStatusMessage(AccountOperationStatus status);
	TransactionDisplay recordToDisplay(const TransactionRecord &rec);
};
