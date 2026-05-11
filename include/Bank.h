#pragma once
#include "Account.h"
#include "Customer.h"
#include "TransactionManager.h"
#include "types.h"
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
	std::shared_mutex bank_mtx;  // To use while adding users/accounts
	std::atomic<uint64_t> next_id{1};
	std::atomic<uint64_t> next_account_id{1};
	std::atomic<uint64_t> next_customer_id{1};

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

	std::vector<TransactionRecord> queryBySrc(uint64_t account_id);

	std::vector<TransactionRecord> queryByDest(uint64_t account_id);

	std::vector<TransactionRecord> queryByAccount(uint64_t account_id);

	std::vector<TransactionRecord>
	queryByTime(std::chrono::system_clock::time_point t1,
	            std::chrono::system_clock::time_point t2);
};