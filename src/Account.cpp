
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
    //withdraw only form active account
    if (state != AccountState::ACTIVE)
        return false;

    if (amount <= 0)
        return false;

    if (balance < amount)
        return false;

    balance -= amount;

    return true;
}

bool Account::credit(int64_t amount)
{
    //deposit in all cases except closed
    if (state == AccountState::CLOSED)
        return false;

    if (amount <= 0)
        return false;

    if (state == AccountState::PENDING)
    {
        if (amount < MINIMUM_PENDING_DEPOSIT)
            return false;

        balance += amount;
        state = AccountState::ACTIVE;

        return true;
    }

    balance += amount;

    return true;
}

AccountOperationStatus Account::freeze()
{
    //freeze only active accounts
    if (state != AccountState::ACTIVE)
    {
        return AccountOperationStatus::FAILED_INVALID_STATE;
    }

    state = AccountState::FROZEN;

    return AccountOperationStatus::OK;
}

AccountOperationStatus Account::unfreeze()
{
    if (state != AccountState::FROZEN)
    {
        return AccountOperationStatus::FAILED_INVALID_STATE;
    }

    state = AccountState::ACTIVE;

    return AccountOperationStatus::OK;
}

AccountOperationStatus Account::close()
{
    if (state != AccountState::ACTIVE &&
        state != AccountState::FROZEN)
    {
        return AccountOperationStatus::FAILED_INVALID_STATE;
    }

    if (balance != 0)
    {
        return AccountOperationStatus::FAILED_BALANCE_NOT_ZERO;
    }

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

