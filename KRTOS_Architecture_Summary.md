# K-RTOS Architecture Analysis Summary (Current Snapshot)

## Project Overview
K-RTOS is a lightweight RTOS for ARM Cortex-M3 (STM32F103C8Tx), supporting preemptive, round-robin, and periodic scheduling. It features dynamic stack allocation, background task support, and efficient context switching using SysTick and PendSV interrupts.

## Key Architecture Components
- **Kernel Core:**
  - TCB with dynamic stack allocation, thread state, and priority
  - Scheduler supports preemptive, round-robin, and periodic modes
  - Context switching via SysTick and PendSV
  - Thread creation, removal, suspension, and background task
- **Time Base System:**
  - SysTick timer (1ms), delays, tick counting
- **Configuration System:**
  - Configurable thread count, stack size, scheduler type
- **System Calls:**
  - Memory management, I/O, process management
- **Hardware Abstraction:**
  - Direct register access, CMSIS, NVIC, SysTick

## Scheduling Architecture
- **Preemptive:** Highest-priority ready task is selected using SysTick and PendSV
- **Round-Robin:** Circular TCB list, equal time slices
- **Periodic:** Tasks run at configurable intervals

## Memory Architecture
- **Flash:** Program code
- **SRAM:** Globals, TCBs, dynamically allocated stacks
- **Stack:** ARM Cortex-M exception frame + saved registers

## Context Switching
- **SysTick_Handler:** Triggers context save and schedules next task
- **PendSV_Handler:** Performs context switch for preemptive multitasking
- **osSchedular_Launch:** Restores context and starts thread

## Interrupt Handling
- **SysTick_IRQn:** Scheduler
- **PendSV_IRQn:** Preemptive context switch
- **Other IRQs:** STM32 peripherals

## Key Features
- Preemptive, round-robin, and periodic scheduling
- Dynamic stack allocation
- Background task support
- Efficient context switching
- Configurable thread count/stack size

## Limitations (Current)
- No priority-based scheduling (planned)
- No advanced resource management (mutexes, semaphores, etc.)
- No stack overflow detection (planned)
- No memory protection (planned)

## Recent Improvements
- Preemptive scheduling and PendSV handler
- Dynamic stack allocation
- Background task support
- Improved thread management

## Conclusion
K-RTOS now supports preemptive multitasking, dynamic stack allocation, and background tasks, providing a robust foundation for embedded real-time applications. Planned improvements include priority scheduling, resource management, and enhanced safety features. 