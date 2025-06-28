# K-RTOS Architecture Analysis

## System Overview

K-RTOS is a lightweight Real-Time Operating System designed for ARM Cortex-M3 microcontrollers, specifically targeting the STM32F103C8Tx platform. The system implements a cooperative and periodic task scheduling mechanism with round-robin scheduling support.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              K-RTOS Architecture                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐         │
│  │   Application   │    │   Application   │    │   Application   │         │
│  │     Layer       │    │     Layer       │    │     Layer       │         │
│  │                 │    │                 │    │                 │         │
│  │  Task0()        │    │  Task1()        │    │  Task2()        │         │
│  │  Task3()        │    │                 │    │                 │         │
│  └─────────────────┘    └─────────────────┘    └─────────────────┘         │
│           │                       │                       │                 │
│           └───────────────────────┼───────────────────────┘                 │
│                                   │                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    K-RTOS Kernel Layer                             │   │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐     │   │
│  │  │   Task Control  │  │   Scheduler     │  │   Time Base     │     │   │
│  │  │   Block (TCB)   │  │                 │  │                 │     │   │
│  │  │                 │  │ • Round Robin   │  │ • SysTick Timer │     │   │
│  │  │ • Stack Mgmt    │  │ • Cooperative   │  │ • Time Slicing  │     │   │
│  │  │ • Context Save  │  │ • Periodic      │  │ • Delay Func    │     │   │
│  │  │ • Thread State  │  │ • Thread Yield  │  │                 │     │   │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────┘     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                   │                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    Hardware Abstraction Layer                      │   │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐     │   │
│  │  │   STM32F103     │  │   CMSIS         │  │   System Calls  │     │   │
│  │  │   Registers     │  │                 │  │                 │     │   │
│  │  │                 │  │ • Core Periph   │  │ • Memory Mgmt   │     │   │
│  │  │ • RCC           │  │ • NVIC          │  │ • I/O Functions │     │   │
│  │  │ • GPIO          │  │ • SysTick       │  │ • File Ops      │     │   │
│  │  │ • SysTick       │  │ • Interrupts    │  │                 │     │   │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────┘     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                   │                                         │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                        Hardware Platform                            │   │
│  │                    STM32F103C8Tx Microcontroller                    │   │
│  │                    ARM Cortex-M3 Core (72MHz)                       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Component Analysis

### 1. Kernel Core (`osKernel.c`)

**Key Components:**
- **Task Control Block (TCB)**: Manages thread state, stack, and scheduling information
- **Scheduler**: Implements round-robin, cooperative, and periodic scheduling
- **Context Switching**: Handles register save/restore during task switches
- **Thread Management**: Creation, removal, and suspension of threads

**Data Structures:**
```c
typedef struct TCB_s {
    int32_t* stack;              // Thread stack pointer
    struct TCB_s* next_thread;   // Next thread in linked list
    callback_function_t callback_function;  // Thread function
    uint32_t stack_size;         // Stack size in words
    uint32_t periodicity;        // For periodic tasks
    uint32_t ex_time;           // Execution time
    int32_t* stackpt;           // Current stack pointer
    uint8_t index;              // Thread index
    uint8_t priority;           // Thread priority
} TCB_config_t;
```

### 2. Time Base System (`Time_Base.c`)

**Features:**
- SysTick timer configuration
- Time-based delays
- Tick counting
- Configurable time periods

**Key Functions:**
- `timebase_init()`: Initialize SysTick timer
- `timebase_ReloadTimeChange()`: Configure timer period
- `Systick_Delay()`: Provide millisecond delays

### 3. Configuration System (`KRTOS_cfg.h`)

**Configurable Parameters:**
- `NUM_OF_THREADS`: Maximum number of threads (3)
- `STACK_SIZE`: Stack size per thread (100 words = 400 bytes)
- `KRTOS_SCHEDULER_TYPE`: Scheduling algorithm selection

### 4. System Calls (`syscalls.c`)

**Implemented System Calls:**
- Memory management (`malloc`, `free`)
- I/O operations (`_read`, `_write`)
- Process management (`_getpid`, `_kill`, `_exit`)
- File operations (stub implementations)

### 5. Startup and Initialization (`startup_stm32f103c8tx.s`)

**Features:**
- Vector table setup
- Stack initialization
- Data/BSS section initialization
- System clock configuration

## Scheduling Architecture

### Round-Robin Scheduler
```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Task 0    │───▶│   Task 1    │───▶│   Task 2    │
│             │    │             │    │             │
└─────────────┘    └─────────────┘    └─────────────┘
       ▲                                    │
       └────────────────────────────────────┘
```

### Periodic Scheduler
```
┌─────────────────────────────────────────────────────────┐
│                Periodic Task Manager                    │
├─────────────────────────────────────────────────────────┤
│  Counter: 0 ──▶ 1 ──▶ 2 ──▶ ... ──▶ 999 ──▶ 0         │
│                                                         │
│  Task3: Execute every 100 ticks                         │
│  Task4: Execute every 200 ticks                         │
│  Task5: Execute every 500 ticks                         │
└─────────────────────────────────────────────────────────┘
```

## Context Switching Mechanism

### Context Save (SysTick_Handler)
1. Disable interrupts (`CPSID I`)
2. Save registers R4-R11 to stack
3. Save current stack pointer to TCB
4. Call periodic scheduler
5. Load next thread's stack pointer
6. Restore registers R4-R11
7. Enable interrupts (`CPSIE I`)
8. Return from exception

### Context Restore (osSchedular_Launch)
1. Load current thread's stack pointer
2. Restore registers R4-R11 from stack
3. Restore registers R0-R3, R12
4. Skip LR and PSR
5. Enable interrupts
6. Branch to thread function

## Memory Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Memory Layout                        │
├─────────────────────────────────────────────────────────┤
│  0x08000000 ──▶ Flash Memory (Program)                 │
│                                                         │
│  0x20000000 ──▶ SRAM (Data + Stacks)                   │
│              ├─▶ Global Variables                       │
│              ├─▶ TCB Array                             │
│              └─▶ Thread Stacks                         │
│                                                         │
│  Stack Layout per Thread:                               │
│  ┌─────────────────┐                                    │
│  │     PSR         │ ← Stack Top                        │
│  │     PC          │                                    │
│  │     LR          │                                    │
│  │     R12         │                                    │
│  │     R3          │                                    │
│  │     R2          │                                    │
│  │     R1          │                                    │
│  │     R0          │                                    │
│  │     R11         │                                    │
│  │     R10         │                                    │
│  │     R9          │                                    │
│  │     R8          │                                    │
│  │     R7          │                                    │
│  │     R6          │                                    │
│  │     R5          │                                    │
│  │     R4          │ ← Stack Pointer                    │
│  └─────────────────┘                                    │
└─────────────────────────────────────────────────────────┘
```

## Interrupt Handling

### Vector Table
- **SysTick_IRQn**: Main scheduler interrupt (priority 15)
- **PendSV_IRQn**: Context switching (if implemented)
- **Other IRQs**: Standard STM32 peripheral interrupts

### Interrupt Priority Levels
- SysTick: Lowest priority (15) for cooperative scheduling
- Other interrupts: Higher priority for real-time responsiveness

## Key Features

### 1. Cooperative Scheduling
- Tasks yield control voluntarily using `osKernel_ThreadYield()`
- No preemption except through SysTick timer
- Simple and predictable behavior

### 2. Periodic Task Support
- Tasks can be configured to run at specific intervals
- Automatic execution based on tick counter
- Configurable periodicity (in milliseconds)

### 3. Stack Management
- Automatic stack allocation for each thread
- Stack initialization with dummy values for debugging
- Proper stack pointer management during context switches

### 4. Configuration Flexibility
- Configurable number of threads
- Adjustable stack sizes
- Selectable scheduling algorithms

## Limitations and Considerations

### 1. Memory Constraints
- Limited to 3 threads maximum
- Fixed stack size per thread (400 bytes)
- No dynamic memory allocation for TCBs

### 2. Scheduling Limitations
- No priority-based scheduling
- No preemptive multitasking
- Limited to round-robin and cooperative modes

### 3. Real-time Constraints
- SysTick-based timing only
- No deadline monitoring
- No resource sharing mechanisms

### 4. Debugging Support
- Basic stack initialization with debug values
- No built-in debugging hooks
- Limited error handling

## Performance Characteristics

- **Context Switch Time**: ~20-30 CPU cycles
- **Memory Overhead**: ~1.2KB for 3 threads
- **Timer Resolution**: 1ms (configurable)
- **Interrupt Latency**: Minimal (direct register access)

## Future Enhancement Opportunities

1. **Priority-based Scheduling**: Implement priority queues
2. **Preemptive Multitasking**: Add PendSV handler
3. **Resource Management**: Mutexes, semaphores, message queues
4. **Memory Protection**: MPU support for Cortex-M3
5. **Power Management**: Low-power modes integration
6. **Debugging Features**: Stack overflow detection, task monitoring
7. **Dynamic Memory**: Heap management for TCBs
8. **Real-time Features**: Deadline monitoring, worst-case execution time analysis 