🌾 Smart Irrigation Node with Real-Time Memory & Task Monitoring
This project implements a FreeRTOS-based Smart Irrigation Node using an Arduino-compatible microcontroller. It simulates intelligent resource-constrained task execution for agricultural applications, with advanced tracking of memory and stack usage per task.

The system is designed around two main concurrent tasks:

SensorTask – simulates data collection from soil/moisture sensors.

CommTask – handles simulated communication with a base station or cloud.

Each task is assigned a memory quota, and dynamic memory allocations are made through a custom wrapper (pvPortMallocWithQuota) that checks whether a task has exceeded its allowed heap usage. Similarly, all deallocations are tracked to maintain per-task memory integrity.

A separate HeapMonitor task continuously logs:

Current free heap space

Minimum ever free heap recorded

Another task, StackMonitor, reports stack usage across all key tasks using uxTaskGetStackHighWaterMark, helping visualize task health and preventing overflow.

🎓 Academic Context
This project was developed during my 4th semester as part of the Operating Systems subject coursework. The goal was to understand:

Real-time task scheduling

Memory management in embedded systems

Inter-task communication and synchronization

System monitoring in resource-constrained environments

By integrating FreeRTOS with hardware simulation, this project helped bridge theoretical OS concepts with practical embedded system implementations.

🔧 Key Technical Features
✅ Per-task dynamic memory quota enforcement

🔐 Mutex-protected memory tracking using SemaphoreHandle_t

📉 Real-time heap usage monitoring via FreeRTOS APIs

🧠 Stack depth monitoring for all active tasks

🕒 Priority-based multitasking simulation for real-world embedded use cases

🌱 Modular and scalable structure suitable for agricultural IoT applications
