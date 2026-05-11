#pragma once
#include "types.h"
#include <cstdint>
#include <mutex>

class Account {
	friend class TransactionManager;

  private:
	uint64_t account_id;  // each account has his account id
	uint64_t customer_id; // each customer has his own id
	int64_t balance;      // customer's balance
	AccountState state;   // check each account's status
	std::mutex mtx;

  public:
	Account(uint64_t account_id, uint64_t customer_id);
	bool debit(int64_t amount);
	bool credit(int64_t amount);
	AccountOperationStatus freeze();   // suspend account
	AccountOperationStatus unfreeze(); // release account again
	AccountOperationStatus close();    // close customer's account
	int64_t getBalance();
	AccountState getState();
	uint64_t getAccountID();
	uint64_t getCustomerID();
};