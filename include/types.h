#pragma once

#include <cstdint>
#include <chrono>

enum TransactionStatus {
    SUCCESS,
    FAILED_INSUFFEICIENT_FUNDS,
    FAILED_ACCOUNT_NOT_FOUND,
    FAILED_ACCOUNT_FROZEN,
    FAILED_INVALID_STATE
};

enum AccountOperationStatus {
    OK,
    FAILED_INVALID_STATE,
    FAILED_BALANCE_NOT_ZERO
};

enum AccountState {
    PENDING,
    ACTIVE,
    FROZEN,
    CLOSED
};

struct TransactionRecord {
    uint64_t transactioId;
    uint64_t src_id;
    uint64_t dest_id;
    int64_t amount;
    TransactionStatus status;
    std::chrono::system_clock::time_point timestamp;
};


