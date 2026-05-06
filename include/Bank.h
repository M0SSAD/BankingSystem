#pragma once
#include "types.h"
#include "Account.h"
#include "Customer.h"
#include <unordered_map>
#include <vector>
#include <memory>

class Bank {
    private:
    TransactionRecord tx_manager;
    std::unordered_map<uint64_t, std::unique_ptr<Account>> accounts;

    public:
    Bank();
    
    AccountState openAccount(uint64_t customer_id);

    AccountState closeAccount(uint64_t account_id);

    AccountState freezeAccount(uint64_t account_id);

    AccountState unfreezeAccount(uint64_t account_id);

    TransactionStatus deposit(uint64_t account_id, uint64_t amount);

    TransactionStatus withdraw(uint64_t account_id, uint64_t amount);

    TransactionStatus transfer(uint64_t account_id, uint64_t amount);

    int64_t getBalance(uint64_t account_id) const;

    std::vector<TransactionRecord> queryBySrc(uint64_t account_id);

    std::vector<TransactionRecord> queryByDest(uint64_t account_id);


};