
#include "Account.h"

using namespace std;

Account::Account(uint64_t account_id, uint64_t customer_id)
    : account_id(account_id),
      customer_id(customer_id),
      balance(0),
      state(AccountState::PENDING)
{
}

bool Account::debit(int64_t amount)
{
    
    if (amount <= 0)
        return false;

    if (balance < amount)
        return false;

    balance -= amount;

    return true;
}

bool Account::credit(uint64_t amount)
{
    balance += amount;

    return true;
}

AccountOperationStatus Account::freeze()
{

    state = AccountState::FROZEN;

    return AccountOperationStatus::OK;
}

AccountOperationStatus Account::unfreeze()
{

    state = AccountState::ACTIVE;

    return AccountOperationStatus::OK;
}

AccountOperationStatus Account::close()
{

    state = AccountState::CLOSED;

    return AccountOperationStatus::OK;
}
//getters
int64_t Account::getBalance()
{
    return balance;
}

AccountState Account::getState()
{
    return state;
}

uint64_t Account::getAccountID()
{
    return account_id;
}

uint64_t Account::getCustomerID()
{
    return customer_id;
}

