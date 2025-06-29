# KRTOS Semaphore Design

## 1. Overview and Purpose
Semaphores provide synchronization and signaling between tasks, supporting both counting and binary (mutex-like) modes.

## 2. Requirements
### Functional
- Support binary and counting semaphores.
- Allow tasks to wait (block) or signal (release) a semaphore.
- Support timeout for wait operations.
### Non-Functional
- Low overhead for signal/wait operations.
- Predictable behavior under high contention.

## 3. Data Structures and Interfaces
- `Semaphore_t`: Structure with count, max count, and waiting queue.
- Interfaces:
  - `osKernel_SemaphoreCreate(Semaphore_t* sem, uint32_t initial_count, uint32_t max_count)`
  - `osKernel_SemaphoreTake(Semaphore_t* sem, uint32_t timeout)`
  - `osKernel_SemaphoreGive(Semaphore_t* sem)`
  - `osKernel_SemaphoreDelete(Semaphore_t* sem)`

## 4. Dynamic Architecture
- State transitions: count decremented on take, incremented on give.
- Tasks blocked if count is zero; unblocked on give.
- FIFO or priority-based waiting queue.

## 5. Integration Constraints
- Must be initialized before use.
- Not suitable for use in ISRs for blocking operations.
- Must handle deletion while tasks are waiting.

## 6. Error Handling
- Return error codes for underflow, overflow, or invalid operations.
- Handle timeout and deletion while waiting.

## 7. Example Use Cases
- Producer-consumer patterns.
- Resource counting (e.g., buffer slots).
- Task synchronization (event signaling).

## 8. Extensibility/Future Enhancements
- Add support for priority-based waiting.
- Add statistics and debugging hooks. 