# K-RTOS Architecture (Current Snapshot)

## System Overview
K-RTOS is a lightweight Real-Time Operating System for ARM Cortex-M3 microcontrollers (STM32F103C8Tx), supporting preemptive, round-robin, and periodic scheduling. It features dynamic stack allocation, background task support, and efficient context switching using SysTick and PendSV interrupts.

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
- **Task Control Block (TCB):**
  - Manages thread state, stack, scheduling info, and priority
  - Supports dynamic stack allocation
- **Scheduler:**
  - Supports preemptive, round-robin, and periodic scheduling
  - Uses SysTick and PendSV for context switching
- **Context Switching:**
  - Register save/restore in assembly
  - PendSV handler for preemptive switches
- **Thread Management:**
  - Thread creation, removal, suspension
  - Background task support

### 2. Time Base System (`Time_Base.c`)
- SysTick timer configuration (1ms resolution)
- Time-based delays and tick counting
- Configurable time periods

### 3. Configuration System (`KRTOS_cfg.h`)
- Configurable thread count and stack size
- Scheduler type selection (preemptive, round-robin, periodic)

### 4. System Calls (`syscalls.c`)
- Memory management (`malloc`, `free`)
- I/O operations (`_read`, `_write`)
- Process management (`_getpid`, `_kill`, `_exit`)

### 5. Startup and Initialization (`startup_stm32f103c8tx.s`)
- Vector table and stack setup
- System clock configuration

## Scheduling Architecture
- **Preemptive Scheduler:** Uses SysTick and PendSV for context switching; highest-priority ready task is selected.
- **Round-Robin Scheduler:** Circular linked list of TCBs; equal time slices.
- **Periodic Scheduler:** Tasks execute at configurable intervals based on tick counter.

## Context Switching Mechanism
- **SysTick_Handler:** Triggers context save and schedules next task.
- **PendSV_Handler:** Performs context switch for preemptive multitasking.
- **osSchedular_Launch:** Restores context and starts thread execution.

## Memory Architecture
- **Flash (0x08000000):** Program code
- **SRAM (0x20000000):** Global variables, TCB array, thread stacks (dynamically allocated)
- **Stack Layout:** Standard ARM Cortex-M exception frame plus saved registers

## Interrupt Handling
- **SysTick_IRQn:** Main scheduler interrupt
- **PendSV_IRQn:** Context switching for preemptive multitasking
- **Other IRQs:** Standard STM32 peripheral interrupts

## Key Features
- Preemptive, round-robin, and periodic scheduling
- Dynamic stack allocation per thread
- Background task support
- Efficient context switching (SysTick + PendSV)
- Configurable thread count and stack size

## Limitations (Current)
- No priority-based scheduling (planned)
- No advanced resource management (mutexes, semaphores, etc.)
- No stack overflow detection (planned)
- No memory protection (planned)

## Recent Improvements
- Added preemptive scheduling and PendSV handler
- Dynamic stack allocation for threads
- Background task support
- Improved thread management and context switching

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