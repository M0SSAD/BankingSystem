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



TransactionStatus TransactionManager::executeTransfer(uint64_t src_id, uint64_t dest_id, int64_t amount){

    if(src_id==dest_id){
        return TransactionStatus::FAILED_INVALID_STATE;
    }

// we need to check that those 2 accounts are exist
auto src_it=accounts_ref.find(src_id);//is this  source account actually exist
auto dest_it=accounts_ref.find(dest_id);//check this destination account actually exist

//if one of them doesn't exist in the system 
 if(src_it==accounts_ref.end() || dest_it==accounts_ref.end()){
    TransactionRecord failed_record{
            // Constructing the record directly
            std::chrono::system_clock::now(),                      // timestamp
            next_tx_id.fetch_add(1, std::memory_order_relaxed),    // transactionId
            src_id,                                                     // src_id
            dest_id,                                            // dest_id
            amount,                                                // amount
            TransactionStatus::FAILED_INVALID_STATE                // status};
};
        writeRecord(failed_record);
              return TransactionStatus::FAILED_INVALID_STATE;}

//create 2 locks to avoid deadlock if the 2 accounts try sending money for each other at the same time 2 accounts should be blocked
//the one that get the lock the one with ascending order ID
std::scoped_lock lock(src_it->second->mtx,dest_it->second->mtx); //the scoped lock ask for the 2 keys for source and destination
//it uses deadlock avoidance alogirthm to grab them safely
//if there is thread using one of the accounts we will wait
//this ensure that in the execute transaction we can safely take 2 accounts perform operation and release them 

//if the source account not active or the destination is closed

if(src_it->second->getState()!=AccountState::ACTIVE||dest_it->second->getState()==AccountState::CLOSED){
//create transaction status that 's invalid
TransactionRecord failed_record{
            // Constructing the record directly
            std::chrono::system_clock::now(),                      // timestamp
            next_tx_id.fetch_add(1, std::memory_order_relaxed),    // transactionId
            src_id,                                                     // src_id
            dest_id,                                            // dest_id
            amount,                                                // amount
            TransactionStatus::FAILED_INVALID_STATE                // status};
};
        writeRecord(failed_record);
              return TransactionStatus::FAILED_INVALID_STATE;}


//check if the source balance has enough money for this transaction?

if(src_it->second->getBalance()<amount){
TransactionRecord failed_record{
            // Constructing the record directly
            std::chrono::system_clock::now(),                      // timestamp
            next_tx_id.fetch_add(1, std::memory_order_relaxed),    // transactionId
            src_id,                                                     // src_id
            dest_id,                                            // dest_id
            amount,                                                // amount
            TransactionStatus::FAILED_INSUFFICIENT_FUNDS               // status};
};
        writeRecord(failed_record);
              return TransactionStatus::FAILED_INSUFFICIENT_FUNDS;}


     src_it->second->debit(amount);
    dest_it->second->credit(amount);         

 TransactionRecord transaction_record {
        std::chrono::system_clock::now(), // timestamp
        next_tx_id.fetch_add(1, std::memory_order_relaxed), // transactionId
        src_id, // src_id
        dest_id, // dest_id
        amount, // amount
        TransactionStatus::SUCCESS, // status
    };   
    writeRecord(transaction_record);  
    return TransactionStatus::SUCCESS;}









