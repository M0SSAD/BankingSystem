#include "TransactionManager.h"

TransactionStatus TransactionManager::executeDeposit(uint64_t account_id, int64_t amount){

    auto acc_it = accounts_ref.find(account_id);
    if(acc_it == accounts_ref.end())
        return TransactionStatus::FAILED_ACCOUNT_NOT_FOUND;

        
        
    std::scoped_lock lock(acc_it->second->mtx); //lock mutex
    if (acc_it->second->getState() == AccountState::CLOSED) {
        // Constructing the record directly
        TransactionRecord failed_record {
            std::chrono::system_clock::now(),                      // timestamp
            next_tx_id.fetch_add(1, std::memory_order_relaxed),    // transactionId
            0,                                                     // src_id
            account_id,                                            // dest_id
            amount,                                                // amount
            TransactionStatus::FAILED_INVALID_STATE                // status
        };
        
        writeRecord(failed_record);
        return TransactionStatus::FAILED_INVALID_STATE;
    }

    if(acc_it->second->getState() == AccountState::PENDING && amount < MINIMUM_PENDING_DEPOSIT){
        TransactionRecord failed_record {
            std::chrono::system_clock::now(),                      // timestamp
            next_tx_id.fetch_add(1, std::memory_order_relaxed),    // transactionId
            0,                                                     // src_id
            account_id,                                            // dest_id
            amount,                                                // amount
            TransactionStatus::FAILED_INVALID_STATE                // status
        };      
        writeRecord(failed_record);  
        return TransactionStatus::FAILED_INVALID_STATE;
    }

    bool status = acc_it->second->credit(amount);

    if(!status) return TransactionStatus::FAILED_INVALID_STATE;


    if(acc_it->second->getState() == AccountState::PENDING)
        acc_it->second->state = AccountState::ACTIVE;

    TransactionRecord transaction_record {
        std::chrono::system_clock::now(), // timestamp
        next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
        0, // src_id
        account_id, // dest_id
        amount, // amount
        TransactionStatus::SUCCESS, // status
    };   
    writeRecord(transaction_record);  
    return TransactionStatus::SUCCESS;
}