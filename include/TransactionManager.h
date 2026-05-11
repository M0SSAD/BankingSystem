#pragma once

#include "Account.h"
#include "types.h"
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class TransactionManager {
  private:
	// Store a reference to the data structure that contains pointers to
	// accounts' object Will get passed by the bank class when it intialize the
	// transaction manager.
	std::unordered_map<uint64_t, std::unique_ptr<Account>> &accounts_ref;
	std::unordered_map<uint64_t, TransactionRecord>
	    transactions; // Stores the transactions with the transaction Id as the
	                  // key.
	std::unordered_multimap<uint64_t, uint64_t>
	    index_by_src; // To store the transaction Id based on the srcId.
	std::unordered_multimap<uint64_t, uint64_t>
	    index_by_dest; // to store the transaction Id based on the destId.
	std::multimap<std::chrono::system_clock::time_point, uint64_t>
	    index_by_time; // sorts the transactions based on their time stamp.
	std::atomic<uint64_t> next_tx_id{1};

	void writeRecord(const TransactionRecord &record);

  public:
	explicit TransactionManager(
	    std::unordered_map<uint64_t, std::unique_ptr<Account>> &accounts)
	    : accounts_ref(accounts) {}

	TransactionStatus executeDeposit(uint64_t account_id, int64_t amount);
	TransactionStatus executeWithdraw(uint64_t account_id, int64_t amount);
	TransactionStatus executeTransfer(uint64_t src_id, uint64_t dest_id,
	                                  int64_t amount);

	AccountOperationStatus executeFreezeAccount(uint64_t account_id);
	AccountOperationStatus executeUnfreezeAccount(uint64_t account_id);
	AccountOperationStatus executeCloseAccount(uint64_t account_id);

	int64_t executeGetBalance(uint64_t account_id);

	std::vector<TransactionRecord>
	queryByTime(std::chrono::system_clock::time_point t1,
	            std::chrono::system_clock::time_point t2);
	std::vector<TransactionRecord> queryBySrc(uint64_t account_id);
	std::vector<TransactionRecord> queryByDest(uint64_t account_id);
};
