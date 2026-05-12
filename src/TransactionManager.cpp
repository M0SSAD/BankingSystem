#include "TransactionManager.h"

TransactionStatus TransactionManager::executeDeposit(uint64_t account_id,
                                                     int64_t amount) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);
	if (amount <= 0) {
		return TransactionStatus::FAILED_INVALID_AMOUNT;
	}

	auto acc_it = accounts_ref.find(account_id);
	if (acc_it == accounts_ref.end())
		return TransactionStatus::FAILED_ACCOUNT_NOT_FOUND;

	// make friend class TransactionManager in Account class to access mtx
	// --private data member lock mutex First
	std::scoped_lock lock(acc_it->second->mtx);
	if (acc_it->second->getState() == AccountState::CLOSED) {
		// Constructing the record directly
		TransactionRecord failed_invalid_record{
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    0,                                                  // src_id
		    account_id,                                         // dest_id
		    amount,                                             // amount
		    TransactionStatus::FAILED_ACCOUNT_CLOSED            // status
		};

		writeRecord(failed_invalid_record);
		return TransactionStatus::FAILED_ACCOUNT_CLOSED;
	}

	if (acc_it->second->getState() == AccountState::PENDING &&
	    amount < MINIMUM_PENDING_DEPOSIT) {
		TransactionRecord failed_invalid_record{
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    0,                                                  // src_id
		    account_id,                                         // dest_id
		    amount,                                             // amount
		    TransactionStatus::FAILED_ACCOUNT_PENDING           // status
		};
		writeRecord(failed_invalid_record);
		return TransactionStatus::FAILED_ACCOUNT_PENDING;
	}

	bool status = acc_it->second->credit(amount);

	if (!status)
		return TransactionStatus::FAILED_INVALID_STATE;

	if (acc_it->second->getState() == AccountState::PENDING)
		acc_it->second->state = AccountState::ACTIVE;

	TransactionRecord successful_record{
	    std::chrono::system_clock::now(),                   // timestamp
	    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
	    0,                                                  // src_id
	    account_id,                                         // dest_id
	    amount,                                             // amount
	    TransactionStatus::SUCCESS,                         // status
	};
	writeRecord(successful_record);
	return TransactionStatus::SUCCESS;
}

TransactionStatus TransactionManager::executeWithdraw(uint64_t account_id,
                                                      int64_t amount) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);
	if (amount <= 0) {
		return TransactionStatus::FAILED_INVALID_AMOUNT;
	}

	auto acc_it = accounts_ref.find(account_id);
	if (acc_it == accounts_ref.end())
		return TransactionStatus::FAILED_ACCOUNT_NOT_FOUND;

	std::scoped_lock lock(acc_it->second->mtx); // lock mutex
	if (acc_it->second->getState() == AccountState::CLOSED ||
	    acc_it->second->getState() == AccountState::PENDING) {
		TransactionRecord failed_invalid_record{
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    0,                                                  // src_id
		    account_id,                                         // dest_id
		    amount,                                             // amount
		    acc_it->second->getState() == AccountState::CLOSED
		        ? TransactionStatus::FAILED_ACCOUNT_CLOSED
		        : TransactionStatus::FAILED_ACCOUNT_PENDING     // status
		};

		writeRecord(failed_invalid_record);
		return acc_it->second->getState() == AccountState::CLOSED
		           ? TransactionStatus::FAILED_ACCOUNT_CLOSED
		           : TransactionStatus::FAILED_ACCOUNT_PENDING;
	}
	if (acc_it->second->getState() == AccountState::FROZEN) {
		// Constructing the record directly
		TransactionRecord failed_frozen_record{
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    0,                                                  // src_id
		    account_id,                                         // dest_id
		    amount,                                             // amount
		    TransactionStatus::FAILED_ACCOUNT_FROZEN            // status
		};

		writeRecord(failed_frozen_record);
		return TransactionStatus::FAILED_ACCOUNT_FROZEN;
	}
	if (acc_it->second->getBalance() < amount) {
		// Constructing the record directly
		TransactionRecord failed_invalid_record{
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    0,                                                  // src_id
		    account_id,                                         // dest_id
		    amount,                                             // amount
		    TransactionStatus::FAILED_INSUFFICIENT_FUNDS        // status
		};

		writeRecord(failed_invalid_record);
		return TransactionStatus::FAILED_INSUFFICIENT_FUNDS;
	}

	bool status = acc_it->second->debit(amount);

	if (!status)
		return TransactionStatus::FAILED_INVALID_STATE;
	TransactionRecord successful_record{
	    std::chrono::system_clock::now(),                   // timestamp
	    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
	    0,                                                  // src_id
	    account_id,                                         // dest_id
	    amount,                                             // amount
	    TransactionStatus::SUCCESS,                         // status
	};

	writeRecord(successful_record);
	return TransactionStatus::SUCCESS;
}

TransactionStatus TransactionManager::executeTransfer(uint64_t src_id,
                                                      uint64_t dest_id,
                                                      int64_t amount) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);

	if (src_id == dest_id) {
		return TransactionStatus::FAILED_SAME_ACCOUNT;
	}
	if (amount <= 0) {
		return TransactionStatus::FAILED_INVALID_AMOUNT;
	}

	// we need to check that those 2 accounts are exist
	auto src_it =
	    accounts_ref.find(src_id); // is this  source account actually exist
	auto dest_it = accounts_ref.find(
	    dest_id); // check this destination account actually exist

	// if one of them doesn't exist in the system
	if (src_it == accounts_ref.end() || dest_it == accounts_ref.end()) {
		TransactionRecord failed_record{
		    // Constructing the record directly
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    src_id,                                             // src_id
		    dest_id,                                            // dest_id
		    amount,                                             // amount
		    TransactionStatus::FAILED_ACCOUNT_NOT_FOUND         // status};
		};
		writeRecord(failed_record);
		return TransactionStatus::FAILED_ACCOUNT_NOT_FOUND;
	}

	// create 2 locks to avoid deadlock if the 2 accounts try sending money for
	// each other at the same time 2 accounts should be blocked the one that get
	// the lock the one with ascending order ID
	std::scoped_lock lock(
	    src_it->second->mtx,
	    dest_it->second->mtx); // the scoped lock ask for the 2 keys for source
	                           // and destination
	// it uses deadlock avoidance alogirthm to grab them safely
	// if there is thread using one of the accounts we will wait
	// this ensure that in the execute transaction we can safely take 2 accounts
	// perform operation and release them

	// if the source account not active or the destination is closed

	if (src_it->second->getState() != AccountState::ACTIVE ||
	    dest_it->second->getState() == AccountState::CLOSED) {
		// create transaction status that 's invalid
		TransactionRecord failed_record{
		    // Constructing the record directly
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    src_id,                                             // src_id
		    dest_id,                                            // dest_id
		    amount,                                             // amount
		    src_it->second->getState() != AccountState::ACTIVE
		        ? TransactionStatus::FAILED_ACCOUNT_NOT_ACTIVE
		        : TransactionStatus::FAILED_ACCOUNT_CLOSED      // status};
		};
		writeRecord(failed_record);
		return src_it->second->getState() != AccountState::ACTIVE
		           ? TransactionStatus::FAILED_ACCOUNT_NOT_ACTIVE
		           : TransactionStatus::FAILED_ACCOUNT_CLOSED;
	}

	// check if the source balance has enough money for this transaction?

	if (src_it->second->getBalance() < amount) {
		TransactionRecord failed_record{
		    // Constructing the record directly
		    std::chrono::system_clock::now(),                   // timestamp
		    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
		    src_id,                                             // src_id
		    dest_id,                                            // dest_id
		    amount,                                             // amount
		    TransactionStatus::FAILED_INSUFFICIENT_FUNDS        // status};
		};
		writeRecord(failed_record);
		return TransactionStatus::FAILED_INSUFFICIENT_FUNDS;
	}

	src_it->second->debit(amount);
	dest_it->second->credit(amount);

	TransactionRecord transaction_record{
	    std::chrono::system_clock::now(),                   // timestamp
	    next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
	    src_id,                                             // src_id
	    dest_id,                                            // dest_id
	    amount,                                             // amount
	    TransactionStatus::SUCCESS,                         // status
	};
	writeRecord(transaction_record);
	return TransactionStatus::SUCCESS;
}

AccountOperationStatus
TransactionManager::executeFreezeAccount(uint64_t account_id) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);
	auto acc_it = accounts_ref.find(account_id);
	if (acc_it == accounts_ref.end()) {
		return AccountOperationStatus::FAILED_ACCOUNT_NOT_FOUND;
	}

	std::scoped_lock lock(acc_it->second->mtx);
	if (acc_it->second->getState() == AccountState::CLOSED ||
	    acc_it->second->getState() == AccountState::PENDING) {
		if (acc_it->second->getState() == AccountState::CLOSED) {
			return AccountOperationStatus::FAILED_ALREADY_CLOSED;
		}
		return AccountOperationStatus::FAILED_ACCOUNT_PENDING;
	}

	if (acc_it->second->getState() == AccountState::ACTIVE) {
		AccountOperationStatus status = acc_it->second->freeze();
		if (status == AccountOperationStatus::OK)
			return AccountOperationStatus::OK;
		return AccountOperationStatus::FAILED_INVALID_STATE;
	}
	if (acc_it->second->getState() == AccountState::FROZEN) {
		return AccountOperationStatus::FAILED_ALREADY_FROZEN;
	}
	return AccountOperationStatus::FAILED_INVALID_STATE;
}

// implement the unfreeze method
AccountOperationStatus
TransactionManager::executeUnfreezeAccount(uint64_t account_id) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);

	auto id = accounts_ref.find(account_id);

	if (id == accounts_ref.end()) {
		// the account doesn't exist
		return AccountOperationStatus::FAILED_ACCOUNT_NOT_FOUND; // this account
		                                                     // doesn't exist
	}
	std::scoped_lock lock(id->second->mtx); // lock this account only 1 thread
	                                        // can use this critical section

	if (id->second->state == AccountState::CLOSED) {
		return AccountOperationStatus::FAILED_ALREADY_CLOSED;
	}
	if (id->second->state == AccountState::ACTIVE) {
		return AccountOperationStatus::FAILED_ALREADY_ACTIVE;
	}
	if (id->second->state != AccountState::FROZEN) {
		return AccountOperationStatus::FAILED_INVALID_STATE;
	}

	if (id->second->state == AccountState::FROZEN) {
		return id->second->unfreeze(); // let this account state change using
		                               // the unfreeze method in account class
	}
	return AccountOperationStatus::FAILED_INVALID_STATE;
}

AccountOperationStatus
TransactionManager::executeCloseAccount(uint64_t account_id) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);
	auto acc_it = accounts_ref.find(account_id);
	if (acc_it == accounts_ref.end()) {
		return AccountOperationStatus::FAILED_ACCOUNT_NOT_FOUND;
	}

	std::scoped_lock lock(acc_it->second->mtx);
	if (acc_it->second->getState() == AccountState::PENDING) {
		return AccountOperationStatus::FAILED_ACCOUNT_PENDING;
	}
	if (acc_it->second->getState() == AccountState::CLOSED) {
		return AccountOperationStatus::FAILED_ALREADY_CLOSED;
	}

	if (acc_it->second->getState() == AccountState::ACTIVE ||
	    acc_it->second->getState() == AccountState::FROZEN) {
		if (acc_it->second->getBalance() != 0)
			return AccountOperationStatus::FAILED_BALANCE_NOT_ZERO;

		AccountOperationStatus status = acc_it->second->close();
		return AccountOperationStatus::OK;
	}
	return AccountOperationStatus::FAILED_INVALID_STATE;
}

// GET BALANCE
int64_t TransactionManager::executeGetBalance(uint64_t account_id) {
	// check account exist or not ?
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);

	auto id = accounts_ref.find(account_id);
	if (id == accounts_ref.end())
		return 0; // nothing exist

	// create mutex
	std::scoped_lock lock(id->second->mtx);
	if (id->second->state == AccountState::CLOSED) {

		return -1; // the account is closed
	}

	return id->second->getBalance();
}

std::optional<AccountState>
TransactionManager::executeGetAccountState(uint64_t account_id) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);
	auto id = accounts_ref.find(account_id);
	if (id == accounts_ref.end()) {
		return std::nullopt;
	}

	std::scoped_lock lock(id->second->mtx);
	return id->second->getState();
}

std::optional<uint64_t>
TransactionManager::executeGetAccountCustomerId(uint64_t account_id) {
	std::shared_lock<std::shared_mutex> bank_lock(bank_mtx);
	auto id = accounts_ref.find(account_id);
	if (id == accounts_ref.end()) {
		return std::nullopt;
	}

	std::scoped_lock lock(id->second->mtx);
	return id->second->getCustomerID();
}

// INDEX BY DESTINATION

std::vector<TransactionRecord>
TransactionManager::queryByDest(uint64_t account_id) {
	std::shared_lock lock(record_mtx);

	std::vector<TransactionRecord> tx_records;
	auto it = history_by_dest.find(account_id);
	if (it == history_by_dest.end()) {
		return {};
	}

	tx_records.reserve(it->second.size());
	for (uint64_t tx_id : it->second) {
		tx_records.push_back(transactions.at(tx_id));
	}

	return tx_records;
}

// INDEX BY SOURCE

std::vector<TransactionRecord>
TransactionManager::queryBySrc(uint64_t account_id) {
	std::shared_lock lock(record_mtx);
	std::vector<TransactionRecord> results;

	auto it = history_by_src.find(account_id);
	if (it == history_by_src.end()) {
		return {};
	}

	results.reserve(it->second.size());
	for (uint64_t tx_id : it->second) {
		results.push_back(transactions.at(tx_id));
	}
	return results;
}

// INDEX BY TIME
std::vector<TransactionRecord>
TransactionManager::queryByTime(std::chrono::system_clock::time_point t1,
                                std::chrono::system_clock::time_point t2) {
	std::shared_lock lock(record_mtx);
	std::vector<TransactionRecord> tx_records;
	auto start_it = index_by_time.lower_bound(t1);
	auto end_it = index_by_time.upper_bound(
	    t2); // changed to upper bound to get an inclusive range.

	for (auto it = start_it; it != end_it; it++) {
		tx_records.push_back(transactions.at(
		    it->second)); // changed the use of [] to .at, because [] is
		                  // mutating, and the shared lock is a read only lock.
	}

	return tx_records;
}

void TransactionManager::writeRecord(const TransactionRecord &record) {
	std::scoped_lock<std::shared_mutex> lock(record_mtx);

	transactions[record.transactionId] = record;
	recent_feed.push_back(record);
	if (recent_feed.size() > recent_capacity) {
		recent_feed.pop_front();
	}

	index_by_time.emplace(record.timestamp, record.transactionId);

	if (record.src_id != 0) {
		history_by_src[record.src_id].push_back(record.transactionId);
	}
	if (record.dest_id != 0) {
		history_by_dest[record.dest_id].push_back(record.transactionId);
	}
}

std::vector<TransactionRecord>
TransactionManager::queryByAccount(uint64_t account_id) {
	std::shared_lock lock(record_mtx);

	static const std::vector<uint64_t> empty;

	auto src_it = history_by_src.find(account_id);
	auto dest_it = history_by_dest.find(account_id);
	const auto &src_ids =
	    (src_it == history_by_src.end()) ? empty : src_it->second;
	const auto &dest_ids =
	    (dest_it == history_by_dest.end()) ? empty : dest_it->second;

	std::vector<TransactionRecord> result;
	result.reserve(src_ids.size() + dest_ids.size());

	size_t i = 0;
	size_t j = 0;
	while (i < src_ids.size() && j < dest_ids.size()) {
		const auto &src_tx = transactions.at(src_ids[i]);
		const auto &dest_tx = transactions.at(dest_ids[j]);

		if (src_tx.timestamp <= dest_tx.timestamp) {
			result.push_back(src_tx);
			++i;
		} else {
			result.push_back(dest_tx);
			++j;
		}
	}

	while (i < src_ids.size()) {
		result.push_back(transactions.at(src_ids[i]));
		++i;
	}
	while (j < dest_ids.size()) {
		result.push_back(transactions.at(dest_ids[j]));
		++j;
	}

	return result;
}

std::vector<size_t> TransactionManager::countTransactionsPerWindow(
    std::chrono::seconds window, std::chrono::seconds step) {
	std::shared_lock<std::shared_mutex> lock(record_mtx);
	std::vector<size_t> counts;
	if (index_by_time.empty()) {
		return counts;
	}

	auto window_start = index_by_time.begin()->first;
	auto window_end = index_by_time.rbegin()->first;
	if (step.count() <= 0 || window.count() <= 0) {
		return counts;
	}

	std::vector<std::chrono::system_clock::time_point> timestamps;
	timestamps.reserve(index_by_time.size());
	for (const auto &entry : index_by_time) {
		timestamps.push_back(entry.first);
	}

	size_t left = 0;
	size_t right = 0;
	for (auto bucket = window_start; bucket <= window_end; bucket += step) {
		auto bucket_end = bucket + window;
		while (right < timestamps.size() && timestamps[right] <= bucket_end) {
			++right;
		}
		while (left < timestamps.size() && timestamps[left] < bucket) {
			++left;
		}
		counts.push_back(right - left);
	}

	return counts;
}

std::vector<TransactionRecord>
TransactionManager::getRecentTransactions(size_t limit) {
	std::shared_lock<std::shared_mutex> lock(record_mtx);
	std::vector<TransactionRecord> result;
	if (limit == 0 || recent_feed.empty()) {
		return result;
	}

	const size_t count = std::min(limit, recent_feed.size());
	result.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		result.push_back(recent_feed[recent_feed.size() - 1 - i]);
	}
	return result;
}
