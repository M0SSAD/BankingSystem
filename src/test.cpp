#include "Bank.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

void runHardTest() {
	Bank myBank;
	uint64_t acc1 = 101;
	uint64_t acc2 = 202;

	std::cout << "--- Starting Hard Test Suite ---" << std::endl;

	// 1. TEST: State Machine Integrity
	myBank.openAccount(acc1);
	// Initial state is PENDING (based on your Enum)
	// Hard Rule: Should not be able to withdraw from a PENDING account
	assert(myBank.withdraw(acc1, 100) ==
	       TransactionStatus::FAILED_INVALID_STATE);
	std::cout << "[PASS] State Machine: Blocked withdrawal on Pending account."
	          << std::endl;

	// 2. TEST: Minimum Deposit Rule
	// You defined MINIMUM_PENDING_DEPOSIT = 500
	assert(myBank.deposit(acc1, 100) ==
	       TransactionStatus::FAILED_INVALID_STATE);
	assert(myBank.deposit(acc1, 600) == TransactionStatus::SUCCESS);
	// Now account should be ACTIVE
	std::cout << "[PASS] Logic: Enforced minimum deposit to activate account."
	          << std::endl;

	// 3. TEST: Frozen Account Logic
	myBank.openAccount(acc2);
	myBank.deposit(acc2, 1000);
	myBank.freezeAccount(acc2);

	// Attempt transfer from frozen account
	assert(myBank.transfer(acc2, acc1, 100) ==
	       TransactionStatus::FAILED_ACCOUNT_FROZEN);
	std::cout << "[PASS] Security: Prevented transfer from Frozen account."
	          << std::endl;

	// 4. TEST: The "Race Condition" Stress Test (Multithreading)
	// We will spawn 10 threads. 5 deposit $10, 5 withdraw $10.
	// The balance should remain exactly the same at the end.
	std::cout << "--- Running Concurrency Stress Test ---" << std::endl;
	int64_t initialBalance = myBank.getBalance(acc1);
	std::vector<std::thread> threads;

	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 0) {
			threads.push_back(std::thread(&Bank::deposit, &myBank, acc1, 10));
		} else {
			threads.push_back(std::thread(&Bank::withdraw, &myBank, acc1, 10));
		}
	}

	for (auto &t : threads)
		t.join();

	assert(myBank.getBalance(acc1) == initialBalance);
	std::cout
	    << "[PASS] Concurrency: Mutexes prevented data corruption under load."
	    << std::endl;

	// 5. TEST: Transaction Indexing
	// Query how many transfers happened from acc1
	auto history = myBank.queryBySrc(acc1);
	assert(history.size() > 0);
	std::cout << "[PASS] Data: Transaction history indexed correctly."
	          << std::endl;

	std::cout << "\nALL HARD TESTS PASSED SUCCESSFULLY!" << std::endl;
}

int main() {
	runHardTest();
	return 0;
}