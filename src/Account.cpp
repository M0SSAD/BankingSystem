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
    lock_guard<mutex> lock(mtx);

    //can withdraw only from active accounts
    if (state != AccountState::ACTIVE)
        return false;

    if (amount <= 0)
        return false;

    if (balance < amount)
        return false;

    balance -= amount;

    return true;
}

bool Account::credit(uint64_t amount)
{
    lock_guard<mutex> lock(mtx);

    //can deposit only if the account is pending or active
    if (state == AccountState::CLOSED ||
        state == AccountState::FROZEN)
    {
        return false;
    }

    if (amount == 0)
        return false;

    balance += amount;

    if (state == AccountState::PENDING &&
        balance >= MINIMUM_PENDING_DEPOSIT)
    {
        state = AccountState::ACTIVE;
    }

    return true;
}

AccountOperationStatus Account::freeze()
{
    lock_guard<mutex> lock(mtx);

   //only freeze an active account
    if (state != AccountState::ACTIVE)
    {
        return AccountOperationStatus::FAILED_INVALID_STATE;
    }

    state = AccountState::FROZEN;

    return AccountOperationStatus::OK;
}

AccountOperationStatus Account::unfreeze()
{
    lock_guard<mutex> lock(mtx);

    if (state != AccountState::FROZEN)
    {
        return AccountOperationStatus::FAILED_INVALID_STATE;
    }

    state = AccountState::ACTIVE;

    return AccountOperationStatus::OK;
}

AccountOperationStatus Account::close()
{
    lock_guard<mutex> lock(mtx);

    if (state == AccountState::CLOSED)
    {
        return AccountOperationStatus::FAILED_INVALID_STATE;
    }

    if (balance != 0)
    {
        //balance must be zero to close the account
        return AccountOperationStatus::FAILED_BALANCE_NOT_ZERO;
    }

    state = AccountState::CLOSED;

    return AccountOperationStatus::OK;
}
//getters

int64_t Account::getBalance()
{
    lock_guard<mutex> lock(mtx);

    return balance;
}

AccountState Account::getState()
{
    lock_guard<mutex> lock(mtx);

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
