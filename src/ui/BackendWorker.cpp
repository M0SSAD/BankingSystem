#include "BackendWorker.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <set>

BackendWorker::BackendWorker(QObject *parent) : QObject(parent) {}

QString BackendWorker::formatTimestamp(
    const std::chrono::system_clock::time_point &tp) {
	auto time_t_val = std::chrono::system_clock::to_time_t(tp);
	std::tm *tm_val = std::localtime(&time_t_val);
	char buf[32];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_val);
	return QString::fromLocal8Bit(buf);
}

QString BackendWorker::txStatusMessage(TransactionStatus status) {
	switch (status) {
	case TransactionStatus::SUCCESS:
		return QStringLiteral("Transaction completed successfully");
	case TransactionStatus::FAILED_INSUFFICIENT_FUNDS:
		return QStringLiteral("Insufficient funds");
	case TransactionStatus::FAILED_ACCOUNT_NOT_FOUND:
		return QStringLiteral("Account not found");
	case TransactionStatus::FAILED_ACCOUNT_FROZEN:
		return QStringLiteral("Account is frozen");
	case TransactionStatus::FAILED_INVALID_STATE:
		return QStringLiteral("Invalid account state");
	case TransactionStatus::FAILED_INVALID_AMOUNT:
		return QStringLiteral("Invalid amount (must be positive)");
	case TransactionStatus::FAILED_ACCOUNT_PENDING:
		return QStringLiteral(
		    "Account is pending (minimum deposit of 500 required)");
	case TransactionStatus::FAILED_ACCOUNT_CLOSED:
		return QStringLiteral("Account is closed");
	case TransactionStatus::FAILED_ACCOUNT_NOT_ACTIVE:
		return QStringLiteral("Source account is not active");
	case TransactionStatus::FAILED_SAME_ACCOUNT:
		return QStringLiteral("Cannot transfer to the same account");
	}
	return QStringLiteral("Unknown error");
}

QString BackendWorker::acctOpStatusMessage(
    AccountOperationStatus status) {
	switch (status) {
	case AccountOperationStatus::OK:
		return QStringLiteral("Operation completed successfully");
	case AccountOperationStatus::FAILED_INVALID_STATE:
		return QStringLiteral("Invalid account state for this operation");
	case AccountOperationStatus::FAILED_BALANCE_NOT_ZERO:
		return QStringLiteral("Cannot close account with non-zero balance");
	case AccountOperationStatus::FAILED_ACCOUNT_PENDING:
		return QStringLiteral("Account is still pending");
	case AccountOperationStatus::FAILED_ACCOUNT_NOT_FOUND:
		return QStringLiteral("Account not found");
	case AccountOperationStatus::FAILED_ALREADY_FROZEN:
		return QStringLiteral("Account is already frozen");
	case AccountOperationStatus::FAILED_ALREADY_ACTIVE:
		return QStringLiteral("Account is already active");
	case AccountOperationStatus::FAILED_ALREADY_CLOSED:
		return QStringLiteral("Account is already closed");
	}
	return QStringLiteral("Unknown error");
}

TransactionDisplay BackendWorker::recordToDisplay(
    const TransactionRecord &rec) {
	TransactionDisplay disp;
	disp.transactionId = rec.transactionId;
	disp.srcId = rec.src_id;
	disp.destId = rec.dest_id;
	disp.amount = rec.amount;
	disp.status = rec.status;
	disp.timestamp = formatTimestamp(rec.timestamp);
	return disp;
}

void BackendWorker::onLookupCustomer(uint64_t nationalId) {
	auto optCustId = m_bank.lookupCustomerByNationalID(nationalId);
	if (!optCustId.has_value()) {
		emit customerLookupResult(false, 0, {}, {});
		return;
	}

	uint64_t cid = optCustId.value();
	auto info = m_bank.getCustomerInfo(cid);
	if (!info.has_value()) {
		emit customerLookupResult(false, 0, {}, {});
		return;
	}

	QString name = QString::fromStdString(info->first);
	auto accIds = m_bank.getCustomerAccountIDs(cid);
	QVector<uint64_t> accounts;
	accounts.reserve(static_cast<int>(accIds.size()));
	for (uint64_t aid : accIds) {
		accounts.append(aid);
	}

	emit customerLookupResult(true, cid, name, accounts);
}

void BackendWorker::onRegisterCustomer(const QString &name,
                                       uint64_t nationalId) {
	uint64_t cid =
	    m_bank.registerCustomer(name.toStdString(), nationalId);
	if (cid != 0) {
		emit operationResult(
		    QStringLiteral(
		        "Customer '%1' registered successfully (ID: %2)")
		        .arg(name)
		        .arg(cid),
		    true);
	} else {
		emit operationResult(
		    QStringLiteral("Failed to register customer"), false);
	}
}

void BackendWorker::onOpenAccount(uint64_t customerId) {
	uint64_t aid = m_bank.openAccount(customerId);
	if (aid != 0) {
		emit operationResult(
		    QStringLiteral("Account opened successfully (ID: %1)")
		        .arg(aid),
		    true);
	} else {
		emit operationResult(
		    QStringLiteral(
		        "Failed to open account. Customer not found."),
		    false);
	}
}

void BackendWorker::onDeposit(uint64_t accountId, int64_t amount) {
	TransactionStatus status = m_bank.deposit(accountId, amount);
	emit operationResult(txStatusMessage(status),
	                     status == TransactionStatus::SUCCESS);
}

void BackendWorker::onWithdraw(uint64_t accountId, int64_t amount) {
	TransactionStatus status = m_bank.withdraw(accountId, amount);
	emit operationResult(txStatusMessage(status),
	                     status == TransactionStatus::SUCCESS);
}

void BackendWorker::onTransfer(uint64_t from, uint64_t to,
                               int64_t amount) {
	TransactionStatus status = m_bank.transfer(from, to, amount);
	emit operationResult(txStatusMessage(status),
	                     status == TransactionStatus::SUCCESS);
}

void BackendWorker::onFreezeAccount(uint64_t accountId) {
	AccountOperationStatus status =
	    m_bank.freezeAccount(accountId);
	emit operationResult(acctOpStatusMessage(status),
	                     status == AccountOperationStatus::OK);
}

void BackendWorker::onUnfreezeAccount(uint64_t accountId) {
	AccountOperationStatus status =
	    m_bank.unfreezeAccount(accountId);
	emit operationResult(acctOpStatusMessage(status),
	                     status == AccountOperationStatus::OK);
}

void BackendWorker::onCloseAccount(uint64_t accountId) {
	AccountOperationStatus status =
	    m_bank.closeAccount(accountId);
	emit operationResult(acctOpStatusMessage(status),
	                     status == AccountOperationStatus::OK);
}

void BackendWorker::onSearchCustomer(const QString &prefix) {
	auto ids =
	    m_bank.searchCustomerPrefix(prefix.toStdString());
	QVector<QPair<uint64_t, QString>> results;
	results.reserve(static_cast<int>(ids.size()));
	for (uint64_t cid : ids) {
		auto info = m_bank.getCustomerInfo(cid);
		if (info.has_value()) {
			results.append(
			    {cid,
			     QString::fromStdString(info->first)});
		}
	}
	emit searchResultsReady(results);
}

void BackendWorker::onRequestAllAccounts() {
	QVector<AccountDisplay> result;
	auto accountIds = m_bank.getAllAccountIDs();

	QMap<uint64_t, QString> customerNames;

	for (uint64_t aid : accountIds) {
		auto state = m_bank.getAccountState(aid);
		AccountState acctState = state.value_or(AccountState::CLOSED);
		if (acctState == AccountState::CLOSED)
			continue;

		AccountDisplay display;
		display.accountId = aid;
		display.balance = m_bank.getBalance(aid);
		display.state = acctState;

		auto custId = m_bank.getAccountCustomerId(aid);
		display.customerId = custId.value_or(0);

		if (display.customerId != 0 &&
		    !customerNames.contains(display.customerId)) {
			auto info = m_bank.getCustomerInfo(display.customerId);
			if (info.has_value()) {
				customerNames[display.customerId] =
				    QString::fromStdString(info->first);
			}
		}
		display.customerName =
		    customerNames.value(display.customerId);

		result.append(display);
	}

	emit accountsReady(result);
}

void BackendWorker::onRequestAllCustomers() {
	QVector<CustomerDisplay> result;
	auto customerIds = m_bank.getAllCustomerIDs();

	for (uint64_t cid : customerIds) {
		CustomerDisplay display;
		display.customerId = cid;

		auto info = m_bank.getCustomerInfo(cid);
		if (!info.has_value())
			continue;
		display.name = QString::fromStdString(info->first);
		display.nationalId = info->second;

		auto accIds = m_bank.getCustomerAccountIDs(cid);
		display.accountCount = static_cast<int>(accIds.size());

		result.append(display);
	}

	emit customersReady(result);
}

void BackendWorker::onRequestAllTransactions() {
	auto records = m_bank.getRecentTransactions(200);
	QVector<TransactionDisplay> result;
	result.reserve(static_cast<int>(records.size()));

	for (const auto &rec : records) {
		result.append(recordToDisplay(rec));
	}

	emit transactionsReady(result);
}

void BackendWorker::onRequestCustomerAccounts(uint64_t customerId) {
	auto accIds = m_bank.getCustomerAccountIDs(customerId);

	QString custName;
	auto info = m_bank.getCustomerInfo(customerId);
	if (info.has_value()) {
		custName = QString::fromStdString(info->first);
	}

	QVector<AccountDisplay> result;
	for (uint64_t aid : accIds) {
		AccountDisplay display;
		display.accountId = aid;
		display.customerId = customerId;
		display.customerName = custName;
		display.balance = m_bank.getBalance(aid);

		auto state = m_bank.getAccountState(aid);
		display.state = state.value_or(AccountState::CLOSED);

		result.append(display);
	}

	emit accountsReady(result);
}

void BackendWorker::onRequestCustomerTransactions(uint64_t customerId) {
	auto accIds = m_bank.getCustomerAccountIDs(customerId);

	std::set<uint64_t> seenTxIds;
	std::vector<TransactionRecord> allRecords;

	for (uint64_t aid : accIds) {
		auto records = m_bank.queryByAccount(aid);
		for (const auto &rec : records) {
			if (seenTxIds.insert(rec.transactionId).second) {
				allRecords.push_back(rec);
			}
		}
	}

	std::sort(allRecords.begin(), allRecords.end(),
	          [](const TransactionRecord &a,
	             const TransactionRecord &b) {
		          return a.timestamp > b.timestamp;
	          });

	QVector<TransactionDisplay> result;
	result.reserve(static_cast<int>(allRecords.size()));
	for (const auto &rec : allRecords) {
		result.append(recordToDisplay(rec));
	}

	emit transactionsReady(result);
}
