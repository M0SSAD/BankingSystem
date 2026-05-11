#include "Bank.h"

Bank::Bank() : tx_manager(accounts) {}

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
