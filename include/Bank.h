#pragma once
#include "Account.h"
#include "Customer.h"
#include "TransactionManager.h"
#include "types.h"
#include <optional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Bank {
  private:
	TransactionManager tx_manager;
	std::unordered_map<uint64_t, std::unique_ptr<Account>> accounts;
	std::unordered_map<uint64_t, std::unique_ptr<Customer>> customers;
	std::unordered_map<uint64_t, uint64_t>
	    national_id_to_customer; // Mapping National IDs to customer objects for
	                             // O(1) Look ups.
	mutable std::shared_mutex bank_mtx;
	std::atomic<uint64_t> next_account_id{1};
	std::atomic<uint64_t> next_customer_id{1};
	std::vector<std::pair<std::string, uint64_t>> name_index;
	bool name_index_sorted = true;

	void ensureNameIndexSorted();
	static std::string toLowerCopy(const std::string &input);

  public:
	Bank();

	uint64_t openAccount(uint64_t customer_id);

	uint64_t registerCustomer(const std::string &name, uint64_t nationalID);

	AccountOperationStatus closeAccount(uint64_t account_id);

	AccountOperationStatus freezeAccount(uint64_t account_id);

	AccountOperationStatus unfreezeAccount(uint64_t account_id);

	TransactionStatus deposit(uint64_t account_id, int64_t amount);

	TransactionStatus withdraw(uint64_t account_id, int64_t amount);

	TransactionStatus transfer(uint64_t account_id1, uint64_t account_id2,
	                           int64_t amount);

	int64_t getBalance(uint64_t account_id);
	std::optional<AccountState> getAccountState(uint64_t account_id);
	std::optional<uint64_t> getAccountCustomerId(uint64_t account_id);

	std::vector<TransactionRecord> queryBySrc(uint64_t account_id);

	std::vector<TransactionRecord> queryByDest(uint64_t account_id);

	std::vector<TransactionRecord> queryByAccount(uint64_t account_id);

	std::vector<uint64_t> searchCustomerPrefix(const std::string &prefix);
	std::vector<size_t>
	countTransactionsPerWindow(std::chrono::seconds window,
	                           std::chrono::seconds step);
	std::vector<TransactionRecord> getRecentTransactions(size_t limit);

	std::vector<TransactionRecord>
	queryByTime(std::chrono::system_clock::time_point t1,
	            std::chrono::system_clock::time_point t2);

	std::optional<uint64_t> lookupCustomerByNationalID(uint64_t national_id) const;
	std::optional<std::pair<std::string, uint64_t>> getCustomerInfo(uint64_t customer_id) const;
	std::vector<uint64_t> getCustomerAccountIDs(uint64_t customer_id) const;
	std::vector<uint64_t> getAllAccountIDs() const;
	std::vector<uint64_t> getAllCustomerIDs() const;
};
