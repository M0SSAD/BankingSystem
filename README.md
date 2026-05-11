# Banking System

A banking system built with C++.
This project implements strict concurrency controls, per-account locking, and a centralized transaction control to guarantee processing without data corruption.

## System Architecture

The `Bank` acts as a public router, while the `TransactionManager` is responsible for the logic.

### Full UML Class Diagram

```mermaid
classDiagram
    class Bank {
        -TransactionManager tx_manager
        -unordered_map~uint64_t, unique_ptr~Account~~ accounts
        +openAccount(customer_id uint64_t) Status
        +closeAccount(account_id uint64_t) Status
        +freezeAccount(account_id uint64_t) Status
        +unfreezeAccount(account_id uint64_t) Status
        +deposit(account_id uint64_t, amount int64_t) Status
        +withdraw(account_id uint64_t, amount int64_t) Status
        +transfer(src_id uint64_t, dest_id uint64_t, amount int64_t) Status
        +getBalance(account_id uint64_t) int64_t
        +queryBySrc(account_id uint64_t) vector~TransactionRecord~
        +queryByDest(account_id uint64_t) vector~TransactionRecord~
    }

    class TransactionManager {
        -unordered_map~uint64_t, unique_ptr~Account~~& accounts_ref
        -map~time_point, TransactionRecord~ index_by_time
        -unordered_multimap~uint64_t, TransactionRecord~ index_by_src
        -unordered_multimap~uint64_t, TransactionRecord~ index_by_dest
        -unordered_map~uint64_t, unique_ptr~mutex~~ account_locks
        -atomic~uint64_t~ next_tx_id
        +executeDeposit(account_id uint64_t, amount int64_t) TransactionStatus
        +executeWithdraw(account_id uint64_t, amount int64_t) TransactionStatus
        +executeTransfer(src_id uint64_t, dest_id uint64_t, amount int64_t) TransactionStatus
        +executeFreezeAccount(account_id uint64_t) AccountOperationStatus
        +executeUnfreezeAccount(account_id uint64_t) AccountOperationStatus
        +executeCloseAccount(account_id uint64_t) AccountOperationStatus
        +executeGetBalance(account_id uint64_t) int64_t
        +queryByTime(t1 time_point, t2 time_point) vector~TransactionRecord~
        +queryBySrc(account_id uint64_t) vector~TransactionRecord~
        +queryByDest(account_id uint64_t) vector~TransactionRecord~
        -writeRecord(record TransactionRecord) void
        -acquireLocks(id1 uint64_t, id2 uint64_t) void
    }

    class Account {
        -uint64_t account_id
        -uint64_t customer_id
        -int64_t balance
        -AccountState state
        +Account(account_id uint64_t, customer_id uint64_t)
        +debit(amount int64_t) bool
        +credit(amount int64_t) bool
        +freeze() AccountOperationStatus
        +unfreeze() AccountOperationStatus
        +close() AccountOperationStatus
        +getBalance() int64_t
        +getState() AccountState
        +getAccountId() uint64_t
        +getCustomerId() uint64_t
    }

    class Customer {
        -string name
        -uint64_t national_id
        -vector~uint64_t~ account_ids
        +Customer(name string, national_id uint64_t)
        +addAccountId(account_id uint64_t) void
        +removeAccountId(account_id uint64_t) void
        +getAccountIds() vector~uint64_t~
        +getName() string
        +getNationalId() uint64_t
    }

    class TransactionRecord {
        <<struct>>
        +uint64_t transaction_id
        +uint64_t src_id
        +uint64_t dest_id
        +int64_t amount
        +TransactionStatus status
        +time_point timestamp
    }

    class AccountState {
        <<enumeration>>
        PENDING
        ACTIVE
        FROZEN
        CLOSED
    }

    class TransactionStatus {
        <<enumeration>>
        SUCCESS
        FAILED_INSUFFICIENT_FUNDS
        FAILED_ACCOUNT_NOT_FOUND
        FAILED_ACCOUNT_FROZEN
        FAILED_INVALID_STATE
    }

    class AccountOperationStatus {
        <<enumeration>>
        OK
        FAILED_INVALID_STATE
        FAILED_BALANCE_NOT_ZERO
    }

    Bank *-- TransactionManager : owns
    Bank *-- Account : owns via unique_ptr
    Bank ..> Customer : serves
    TransactionManager ..> Account : mutates via ref
    TransactionManager *-- TransactionRecord : owns
    Account *-- AccountState : has
    TransactionRecord *-- TransactionStatus : has
    Account ..> AccountOperationStatus : returns
    TransactionManager ..> AccountOperationStatus : returns
```

---

## Lifecycle & Flow

### Account State Machine

Accounts strictly follow this lifecycle.

```mermaid
stateDiagram-v2
    [*] --> PENDING : Account Opened
    PENDING --> ACTIVE : Min deposit met
    ACTIVE --> FROZEN : Freeze command
    ACTIVE --> CLOSED : Close request
    FROZEN --> ACTIVE : Unfreeze command
    FROZEN --> CLOSED : Close request
    CLOSED --> [*] : Terminal State

    note right of PENDING : Deposit (>= min) only
    note right of ACTIVE : Fully operational
    note right of FROZEN : Deposit & Query only
    note right of CLOSED : History Read-only
```

### The Transfer Call Chain

```mermaid
flowchart TD
    A[1. Request arrives at Bank\nPure router] --> B[2. Bank delegates to TxManager\nPasses IDs + amount]
    B --> C[3. Acquire locks\nAscending Account ID order]
    C --> D[4. Validate inside lock scope\ngetState<br> == ACTIVE? getBalance<br> >= amount?]
    D --> E{Valid?}
    E -- NO --> F[Release locks\nWrite FAILED_* TransactionRecord]
    E -- YES --> G[Execute\ndebit<br> on src, credit<br> on dest]
    G --> H[Release locks\nWrite SUCCESS TransactionRecord]
    F --> I[5. Return TransactionStatus to caller]
    H --> I
```

## STRICT RULES

1. **Deadlock Prevention (Lock Ordering):** In any multi-account operation, mutexes are **always** acquired in ascending `uint64_t` Account ID order. No exceptions.
2. **Atomic Validation:** Validation always occurs *inside* the lock scope, never before acquiring the lock.
3. **Single Source of Truth:** `TransactionHistory` is an internal container inside `TransactionManager`, not a separate entity.
4. **Separation of Concerns:** `Bank` performs zero business logic. It routes and owns. Only `TransactionManager` mutates `Account` state.
5. **Memory Safety:** `Customer` objects hold Account IDs only. No raw pointers or smart pointers to `Account` objects.
6. **Audit Integrity:** Failed transactions produce exactly one `FAILED_*` `TransactionRecord`. No partial records. No silent failures.
7. **Thread-Safe IDs:** Transaction IDs are unique `uint64_t` values generated with `std::atomic<uint64_t>`.
