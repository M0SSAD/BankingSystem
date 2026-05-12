#pragma once

#include "Account.h"
#include "types.h"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class TransactionManager {
  private:
	// Store a reference to the data structure that contains pointers to
	// accounts' object Will get passed by the bank class when it intialize the
	// transaction manager.
	std::unordered_map<uint64_t, std::unique_ptr<Account>> &accounts_ref;
	std::shared_mutex &bank_mtx;
	std::unordered_map<uint64_t, TransactionRecord>
	    transactions; // Stores the transactions with the transaction Id as the
	                  // key.
	std::unordered_map<uint64_t, std::vector<uint64_t>>
	    history_by_src; // Store transaction Ids based on the srcId.
	std::unordered_map<uint64_t, std::vector<uint64_t>>
	    history_by_dest; // Store transaction Ids based on the destId.
	std::multimap<std::chrono::system_clock::time_point, uint64_t>
	    index_by_time; // sorts the transactions based on their time stamp.

	std::shared_mutex record_mtx; // Mutex to use while writing records, so only
	                              // one thread can store the record at a time.
	std::atomic<uint64_t> next_tx_id{1};
	std::deque<TransactionRecord> recent_feed;
	size_t recent_capacity = 200;

	void writeRecord(const TransactionRecord &record);

  public:
	explicit TransactionManager(
	    std::unordered_map<uint64_t, std::unique_ptr<Account>> &accounts,
	    std::shared_mutex &bankMTX)
	    : accounts_ref(accounts), bank_mtx(bankMTX) {}

	TransactionStatus executeDeposit(uint64_t account_id, int64_t amount);
	TransactionStatus executeWithdraw(uint64_t account_id, int64_t amount);
	TransactionStatus executeTransfer(uint64_t src_id, uint64_t dest_id,
	                                  int64_t amount);

	AccountOperationStatus executeFreezeAccount(uint64_t account_id);
	AccountOperationStatus executeUnfreezeAccount(uint64_t account_id);
	AccountOperationStatus executeCloseAccount(uint64_t account_id);

	int64_t executeGetBalance(uint64_t account_id);
	std::optional<AccountState> executeGetAccountState(uint64_t account_id);
	std::optional<uint64_t> executeGetAccountCustomerId(uint64_t account_id);

	std::vector<TransactionRecord>
	queryByTime(std::chrono::system_clock::time_point t1,
	            std::chrono::system_clock::time_point t2);
	std::vector<TransactionRecord> queryBySrc(uint64_t account_id);
	std::vector<TransactionRecord> queryByDest(uint64_t account_id);
	std::vector<TransactionRecord> queryByAccount(uint64_t account_id);
	std::vector<size_t>
	countTransactionsPerWindow(std::chrono::seconds window,
	                           std::chrono::seconds step);
	std::vector<TransactionRecord> getRecentTransactions(size_t limit);
};
