#pragma once
#include "types.h"
#include "Account.h"
#include "Customer.h"
#include "TransactionManager.h"
#include <unordered_map>
#include <vector>
#include <memory>

class Bank {
    private:
    TransactionManager tx_manager;
    std::unordered_map<uint64_t, std::unique_ptr<Account>> accounts;

    std::atomic<uint64_t> next_id;

    public:
    Bank();
    
    AccountOperationStatus openAccount(uint64_t account_id);

    AccountOperationStatus closeAccount(uint64_t account_id);

    AccountOperationStatus freezeAccount(uint64_t account_id);

    AccountOperationStatus unfreezeAccount(uint64_t account_id);

    TransactionStatus deposit(uint64_t account_id, uint64_t amount);

    TransactionStatus withdraw(uint64_t account_id, uint64_t amount);

    TransactionStatus transfer(uint64_t account_id1, uint64_t account_id2,uint64_t amount);

    int64_t getBalance(uint64_t account_id);

    std::vector<TransactionRecord> queryBySrc(uint64_t account_id);

    std::vector<TransactionRecord> queryByDest(uint64_t account_id);

    std::vector<TransactionRecord> queryByTime(std::chrono::system_clock::time_point t1, std::chrono::system_clock::time_point t2);


};