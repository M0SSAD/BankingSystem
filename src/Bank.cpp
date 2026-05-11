#include "Bank.h"

Bank::Bank() {
	// To DO
}

uint64_t Bank::openAccount(uint64_t account_id) {
	// TO DO
}

uint64_t Bank::registerCustomer(const std::string &name, uint64_t nationalID) {
	// TO DO
}

AccountOperationStatus Bank::closeAccount(uint64_t account_id) {
	return tx_manager.executeCloseAccount(account_id);
}

AccountOperationStatus Bank::freezeAccount(uint64_t account_id) {
	return tx_manager.executeFreezeAccount(account_id);
}

AccountOperationStatus Bank::unfreezeAccount(uint64_t account_id) {
	return tx_manager.executeCloseAccount(account_id);
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
	// To Do
}
