#include "TransactionManager.h"

TransactionStatus TransactionManager::executeDeposit(uint64_t account_id, int64_t amount){

    auto acc_it = accounts_ref.find(account_id);
    if(acc_it == accounts_ref.end())
        return TransactionStatus::FAILED_ACCOUNT_NOT_FOUND;

    if (acc_it->second->getState() == AccountState::CLOSED) {
    // Constructing the record directly
    TransactionRecord failed_record {
        .transactioId = next_tx_id.fetch_add(1, std::memory_order_relaxed),
        .src_id = 0, 
        .dest_id = account_id,
        .amount = amount,
        .status = TransactionStatus::FAILED_INVALID_STATE,
        .timestamp = std::chrono::system_clock::now()
    };
    
    writeRecord(failed_record);
    return TransactionStatus::FAILED_INVALID_STATE;
}

    if(acc_it->second->getState() == AccountState::PENDING && amount < MINMUM_PENDING_DEPOSIT){
        TransactionRecord failed_record {
        .transactioId = next_tx_id.fetch_add(1, std::memory_order_relaxed),
        .src_id = 0, 
        .dest_id = account_id,
        .amount = amount,
        .status = TransactionStatus::FAILED_INVALID_STATE,
        .timestamp = std::chrono::system_clock::now()
    };        
        writeRecord(failed_record);  
        return TransactionStatus::FAILED_INSUFFEICIENT_FUNDS;
    }
        

    std::scoped_lock lock(acc_it->second->mtx); //lock mutex

    bool status = acc_it->second->credit(amount);

    if(!status) return TransactionStatus::FAILED_INVALID_STATE;


    if(acc_it->second->getState() == AccountState::PENDING)
        acc_it->second->state = AccountState::ACTIVE;

    TransactionRecord transaction_record {
        .transactioId = next_tx_id.fetch_add(1, std::memory_order_relaxed),
        .src_id = 0, 
        .dest_id = account_id,
        .amount = amount,
        .status = TransactionStatus::SUCCESS,
        .timestamp = std::chrono::system_clock::now()   
    };   
        writeRecord(transaction_record);  
        return TransactionStatus::SUCCESS;

}