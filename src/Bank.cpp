#include "Bank.h"

#include <algorithm>
#include <cctype>

Bank::Bank() : tx_manager(accounts, bank_mtx) {}

std::string Bank::toLowerCopy(const std::string &input) {
	std::string out;
	out.reserve(input.size());
	for (unsigned char c : input) {
		out.push_back(static_cast<char>(std::tolower(c)));
	}
	return out;
}

void Bank::ensureNameIndexSorted() {
	if (name_index_sorted) {
		return;
	}
	std::sort(name_index.begin(), name_index.end(),
	          [](const auto &a, const auto &b) { return a.first < b.first; });
	name_index_sorted = true;
}

uint64_t Bank::openAccount(uint64_t customerId) {
	// acquire the lock
	std::scoped_lock lock(bank_mtx);
	// check that the customer exists
	auto it = customers.find(customerId);
	if (it == customers.end()) {
		return 0;
	}
	// generate a new_id
	uint64_t new_id = next_account_id.fetch_add(1, std::memory_order_relaxed);
	// make an account and take a unique pointer to it
	auto account = std::make_unique<Account>(new_id, customerId);
	// add the account Id to the customer.
	bool success = it->second->addAccountID(new_id);
	accounts[new_id] = std::move(account);
	return new_id;
}

uint64_t Bank::registerCustomer(const std::string &name, uint64_t nationalID) {
	std::scoped_lock lock(bank_mtx);
	// make sure that the nationalId is unique.
	auto it = national_id_to_customer.find(nationalID);
	if (it != national_id_to_customer.end()) {
		return it->second; // if they exit return their ID.
	}
	// generate the Id
	uint64_t new_id = next_customer_id.fetch_add(1, std::memory_order_relaxed);
	// make the customer object and take a unique ptr to it
	auto customer = std::make_unique<Customer>(name, nationalID);
	// save the data
	national_id_to_customer.insert({nationalID, new_id});
	customers[new_id] = std::move(customer);
	name_index.emplace_back(toLowerCopy(name), new_id);
	name_index_sorted = false;
	return new_id;
}

AccountOperationStatus Bank::closeAccount(uint64_t account_id) {
	return tx_manager.executeCloseAccount(account_id);
}

AccountOperationStatus Bank::freezeAccount(uint64_t account_id) {
	return tx_manager.executeFreezeAccount(account_id);
}

AccountOperationStatus Bank::unfreezeAccount(uint64_t account_id) {
	return tx_manager.executeUnfreezeAccount(account_id);
}

TransactionStatus Bank::deposit(uint64_t account_id, int64_t amount) {
	return tx_manager.executeDeposit(account_id, amount);
}

TransactionStatus Bank::withdraw(uint64_t account_id, int64_t amount) {
	return tx_manager.executeWithdraw(account_id, amount);
}

TransactionStatus Bank::transfer(uint64_t account_id1, uint64_t account_id2,
                                 int64_t amount) {
	return tx_manager.executeTransfer(account_id1, account_id2, amount);
}

int64_t Bank::getBalance(uint64_t account_id) {
	return tx_manager.executeGetBalance(account_id);
}

std::optional<AccountState> Bank::getAccountState(uint64_t account_id) {
	return tx_manager.executeGetAccountState(account_id);
}

std::optional<uint64_t> Bank::getAccountCustomerId(uint64_t account_id) {
	return tx_manager.executeGetAccountCustomerId(account_id);
}

std::vector<TransactionRecord>
Bank::queryByTime(std::chrono::system_clock::time_point t1,
                  std::chrono::system_clock::time_point t2) {
	return tx_manager.queryByTime(t1, t2);
}

std::vector<TransactionRecord> Bank::queryBySrc(uint64_t account_id) {
	return tx_manager.queryBySrc(account_id);
}

std::vector<TransactionRecord> Bank::queryByDest(uint64_t account_id) {
	return tx_manager.queryByDest(account_id);
}

std::vector<TransactionRecord> Bank::queryByAccount(uint64_t account_id) {
	return tx_manager.queryByAccount(account_id);
}

std::vector<uint64_t> Bank::searchCustomerPrefix(const std::string &prefix) {
	std::scoped_lock lock(bank_mtx);
	ensureNameIndexSorted();

	std::vector<uint64_t> results;
	if (prefix.empty() || name_index.empty()) {
		return results;
	}
	const std::string lower_prefix = toLowerCopy(prefix);
	const std::string upper_prefix = lower_prefix + char(0x7F);

	auto lower = std::lower_bound(
	    name_index.begin(), name_index.end(), lower_prefix,
	    [](const auto &entry, const std::string &value) {
		    return entry.first < value;
	    });
	auto upper = std::upper_bound(
	    name_index.begin(), name_index.end(), upper_prefix,
	    [](const std::string &value, const auto &entry) {
		    return value < entry.first;
	    });

	for (auto it = lower; it != upper; ++it) {
		results.push_back(it->second);
	}
	return results;
}

std::vector<size_t>
Bank::countTransactionsPerWindow(std::chrono::seconds window,
	                               std::chrono::seconds step) {
	return tx_manager.countTransactionsPerWindow(window, step);
}

std::vector<TransactionRecord> Bank::getRecentTransactions(size_t limit) {
	return tx_manager.getRecentTransactions(limit);
}

std::optional<uint64_t>
Bank::lookupCustomerByNationalID(uint64_t national_id) const {
	std::shared_lock lock(bank_mtx);
	auto it = national_id_to_customer.find(national_id);
	if (it != national_id_to_customer.end()) {
		return it->second;
	}
	return std::nullopt;
}

std::optional<std::pair<std::string, uint64_t>>
Bank::getCustomerInfo(uint64_t customer_id) const {
	std::shared_lock lock(bank_mtx);
	auto it = customers.find(customer_id);
	if (it != customers.end()) {
		return std::make_pair(it->second->getName(),
		                      it->second->getNationalID());
	}
	return std::nullopt;
}

std::vector<uint64_t> Bank::getCustomerAccountIDs(uint64_t customer_id) const {
	std::shared_lock lock(bank_mtx);
	auto it = customers.find(customer_id);
	if (it != customers.end()) {
		return it->second->getAccountIDs();
	}
	return {};
}

std::vector<uint64_t> Bank::getAllAccountIDs() const {
	std::shared_lock lock(bank_mtx);
	std::vector<uint64_t> ids;
	ids.reserve(accounts.size());
	for (const auto &[id, _] : accounts) {
		ids.push_back(id);
	}
	return ids;
}

std::vector<uint64_t> Bank::getAllCustomerIDs() const {
	std::shared_lock lock(bank_mtx);
	std::vector<uint64_t> ids;
	ids.reserve(customers.size());
	for (const auto &[id, _] : customers) {
		ids.push_back(id);
	}
	return ids;
}
