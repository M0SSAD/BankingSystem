#pragma once

#include <cstdint>
#include <chrono>

const int64_t MINIMUM_PENDING_DEPOSIT = 500;

enum class TransactionStatus : uint8_t {
	SUCCESS,
	FAILED_INSUFFICIENT_FUNDS,
	FAILED_ACCOUNT_NOT_FOUND,
	FAILED_ACCOUNT_FROZEN,
	FAILED_INVALID_STATE,
	FAILED_INVALID_AMOUNT,
	FAILED_ACCOUNT_PENDING,
	FAILED_ACCOUNT_CLOSED,
	FAILED_ACCOUNT_NOT_ACTIVE,
	FAILED_SAME_ACCOUNT
};

enum class AccountOperationStatus : uint8_t {
	OK,
	FAILED_INVALID_STATE,
	FAILED_BALANCE_NOT_ZERO,
	FAILED_ACCOUNT_PENDING,
	FAILED_ACCOUNT_NOT_FOUND,
	FAILED_ALREADY_FROZEN,
	FAILED_ALREADY_ACTIVE,
	FAILED_ALREADY_CLOSED
};

enum class AccountState : uint8_t {
    PENDING,
    ACTIVE,
    FROZEN,
    CLOSED
};

struct TransactionRecord {
    std::chrono::system_clock::time_point timestamp;
    uint64_t transactionId;
    uint64_t src_id;
    uint64_t dest_id;
    int64_t amount;
    TransactionStatus status;
};
