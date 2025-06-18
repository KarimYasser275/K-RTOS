# K-RTOS Architecture Analysis Summary

## Project Overview

K-RTOS is a lightweight Real-Time Operating System designed for ARM Cortex-M3 microcontrollers, specifically targeting the STM32F103C8Tx platform. The system implements a cooperative and periodic task scheduling mechanism with round-robin scheduling support.

## Key Architecture Components

### 1. **Kernel Core (`osKernel.c`)**
- **Task Control Block (TCB) Management**: Handles thread state, stack allocation, and scheduling information
- **Scheduler Implementation**: Supports round-robin, cooperative, and periodic scheduling
- **Context Switching**: Efficient register save/restore mechanism using assembly code
- **Thread Management**: Creation, removal, and suspension capabilities

### 2. **Time Base System (`Time_Base.c`)**
- **SysTick Timer Configuration**: Provides system timing with 1ms resolution
- **Time-based Delays**: Implements millisecond delay functions
- **Tick Counting**: Maintains system tick counter for periodic tasks
- **Configurable Periods**: Supports adjustable timer periods

### 3. **Configuration System (`KRTOS_cfg.h`)**
- **Thread Limits**: Maximum 3 threads supported
- **Stack Configuration**: 100 words (400 bytes) per thread
- **Scheduling Selection**: Configurable scheduling algorithms

### 4. **System Calls (`syscalls.c`)**
- **Memory Management**: `malloc()` and `free()` implementations
- **I/O Operations**: `_read()` and `_write()` functions
- **Process Management**: Basic process control functions
- **File Operations**: Stub implementations for compatibility

### 5. **Hardware Abstraction**
- **STM32F103 Registers**: Direct register access for GPIO, SysTick, RCC
- **CMSIS Integration**: Standard ARM Cortex-M3 peripheral access
- **Interrupt Management**: NVIC configuration and exception handling

## Scheduling Architecture

### **Round-Robin Scheduler**
- Circular linked list of TCBs
- Equal time slices for all tasks
- Automatic task switching via SysTick interrupt

### **Cooperative Scheduler**
- Tasks voluntarily yield control using `osKernel_ThreadYield()`
- No preemption except through timer interrupts
- Predictable execution order

### **Periodic Scheduler**
- Tasks execute at configurable intervals
- Automatic execution based on tick counter
- Supports multiple periodic tasks with different periods

## Memory Architecture

### **Memory Layout**
- **Flash Memory (0x08000000)**: Program code, vector table, constants
- **SRAM (0x20000000)**: Global variables, TCB array, thread stacks, heap

### **Stack Management**
- **Per-thread Stacks**: 400 bytes each (100 words)
- **Stack Initialization**: Debug values (0xAAAAAAAA) for easy identification
- **Context Save/Restore**: Efficient register management during switches

## Context Switching Mechanism

### **Context Save (SysTick_Handler)**
1. Disable interrupts (`CPSID I`)
2. Save registers R4-R11 to current stack
3. Save current stack pointer to TCB
4. Call periodic scheduler
5. Load next thread's stack pointer
6. Restore registers R4-R11
7. Enable interrupts (`CPSIE I`)
8. Return from exception

### **Context Restore (osSchedular_Launch)**
1. Load current thread's stack pointer
2. Restore registers R4-R11 from stack
3. Restore registers R0-R3, R12
4. Skip LR and PSR
5. Enable interrupts
6. Branch to thread function

## Interrupt Handling

### **Vector Table**
- **SysTick_IRQn**: Main scheduler interrupt (priority 15)
- **PendSV_IRQn**: Context switching (if implemented)
- **Other IRQs**: Standard STM32 peripheral interrupts

### **Interrupt Priority**
- SysTick: Lowest priority for cooperative scheduling
- Other interrupts: Higher priority for real-time responsiveness

## Key Features

### **1. Cooperative Scheduling**
- Voluntary task yielding
- Simple and predictable behavior
- Low overhead context switching

### **2. Periodic Task Support**
- Configurable execution intervals
- Automatic task execution
- Multiple periodic tasks support

### **3. Stack Management**
- Automatic stack allocation
- Debug-friendly initialization
- Proper stack pointer management

### **4. Configuration Flexibility**
- Adjustable thread count
- Configurable stack sizes
- Selectable scheduling algorithms

## Performance Characteristics

- **Context Switch Time**: ~20-30 CPU cycles
- **Memory Overhead**: ~1.2KB for 3 threads
- **Timer Resolution**: 1ms (configurable)
- **Interrupt Latency**: Minimal (direct register access)

## Limitations

### **1. Memory Constraints**
- Limited to 3 threads maximum
- Fixed stack size per thread
- No dynamic TCB allocation

### **2. Scheduling Limitations**
- No priority-based scheduling
- No preemptive multitasking
- Limited scheduling algorithms

### **3. Real-time Constraints**
- SysTick-based timing only
- No deadline monitoring
- No resource sharing mechanisms

### **4. Debugging Support**
- Basic stack initialization
- No built-in debugging hooks
- Limited error handling

## Future Enhancement Opportunities

1. **Priority-based Scheduling**: Implement priority queues
2. **Preemptive Multitasking**: Add PendSV handler
3. **Resource Management**: Mutexes, semaphores, message queues
4. **Memory Protection**: MPU support for Cortex-M3
5. **Power Management**: Low-power modes integration
6. **Debugging Features**: Stack overflow detection, task monitoring
7. **Dynamic Memory**: Heap management for TCBs
8. **Real-time Features**: Deadline monitoring, WCET analysis

## Code Structure Analysis

### **File Organization**
```
RTOS/
├── KRTOS/
│   ├── inc/osKernel.h          # Kernel API definitions
│   └── src/osKernel.c          # Kernel implementation
├── Inc/
│   ├── KRTOS_cfg.h             # Configuration parameters
│   ├── Time_Base.h             # Time base API
│   └── CMSIS/                  # ARM CMSIS headers
├── Src/
│   ├── main.c                  # Application entry point
│   ├── Time_Base.c             # Time base implementation
│   ├── syscalls.c              # System call implementations
│   └── KRTOS_cfg.c             # Configuration implementation
└── Startup/
    └── startup_stm32f103c8tx.s # System startup and vector table
```

### **Key Data Structures**
```c
// Task Control Block
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
} TCB_t;

// Simplified TCB for internal use
typedef struct tcb {
    int32_t *stackPt;           // Stack pointer
    struct tcb *nextPt;         // Next TCB in list
} TCBType;

// Periodic task structure
typedef struct TCB_peridoic_s {
    callback_function_t functionPt;  // Task function
    uint32_t periodicity;           // Execution period
} TCB_peridoic_t;
```

## Conclusion

K-RTOS represents a well-designed lightweight RTOS for resource-constrained ARM Cortex-M3 microcontrollers. The architecture demonstrates good separation of concerns with clear layering between application, kernel, and hardware abstraction layers. The system provides essential RTOS functionality while maintaining simplicity and efficiency suitable for embedded applications.

The cooperative scheduling approach ensures predictable behavior, while the periodic task support enables time-critical operations. The efficient context switching mechanism and minimal memory footprint make it suitable for applications with strict resource constraints.

While the current implementation has some limitations in terms of scalability and advanced features, it provides a solid foundation that can be extended with additional functionality as requirements evolve. 