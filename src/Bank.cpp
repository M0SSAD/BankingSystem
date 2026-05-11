#include "Bank.h"

Bank::Bank() : tx_manager(accounts), next_id(1000) {}          // Start account IDs at 1000 (or 1)

AccountOperationStatus Bank::openAccount(uint64_t account_id){
    //lock incrementing id_s
    next_id.fetch_add(1,std::memory_order_relaxed);
    //accounts[account_id];
    //Account Object itself created!!
}

AccountOperationStatus Bank::closeAccount(uint64_t account_id){
    return tx_manager.executeCloseAccount(account_id);
}

AccountOperationStatus Bank::freezeAccount(uint64_t account_id){
    return tx_manager.executeFreezeAccount(account_id);
}

AccountOperationStatus Bank::unfreezeAccount(uint64_t account_id){
    return tx_manager.executeCloseAccount(account_id);
}

TransactionStatus Bank::deposit(uint64_t account_id, uint64_t amount){
    return tx_manager.executeDeposit(account_id, amount);
}

TransactionStatus Bank::withdraw(uint64_t account_id, uint64_t amount){
    return tx_manager.executeWithdraw(account_id, amount);
}

TransactionStatus Bank::transfer(uint64_t account_id1, uint64_t account_id2,uint64_t amount){
    return tx_manager.executeTransfer(account_id1, account_id2, amount);
}

int64_t Bank::getBalance(uint64_t account_id){
    return tx_manager.executeGetBalance(account_id);
}

std::vector<TransactionRecord> Bank::queryByTime    (std::chrono::system_clock::time_point t1, std::chrono::system_clock::time_point t2){
    return tx_manager.queryByTime(t1, t2);

}

std::vector<TransactionRecord> Bank::queryBySrc(uint64_t account_id){
    return tx_manager.queryBySrc(account_id);
}

std::vector<TransactionRecord> Bank::queryByDest(uint64_t account_id){
    return tx_manager.queryByDest(account_id);
}
