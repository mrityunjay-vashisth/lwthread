# LWThread - Lightweight Threading Library

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/version-0.1.0-orange.svg)]()

A lightweight threading library inspired by Go's goroutines, implemented in C. LWThread provides an efficient M:N threading model that maps many user-space threads to fewer OS threads, enabling highly concurrent applications with minimal overhead.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [Architecture](#architecture)
- [Developer Guide](#developer-guide)
  - [Understanding the Code](#understanding-the-code)
  - [Threading Model](#threading-model)
  - [Design Patterns](#design-patterns)
  - [Extending the Library](#extending-the-library)
- [Performance Considerations](#performance-considerations)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Lightweight Threads**: Create thousands of threads with minimal overhead (2KB stack per thread)
- **M:N Threading Model**: Many user-space threads mapped to fewer OS threads
- **Cooperative Scheduling**: Thread yields for cooperative multitasking
- **Simple API**: Easy-to-use interface for creating and managing threads
- **Cross-Platform**: Works on Linux, macOS, and other POSIX-compliant systems
- **Entity-based Concurrency**: Advanced patterns for entity-based concurrency control

## Requirements

- C11-compatible compiler (GCC, Clang)
- POSIX-compliant system (Linux, macOS, BSD)
- CMake 3.10 or newer
- pthread library
- libcurl (optional, for web crawler example)

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/lwthread.git
cd lwthread

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests (optional)
make test

# Install (optional)
sudo make install
```

## Quick Start

Here's a minimal example to get you started:

```c
#include <lwthread/lwthread.h>
#include <stdio.h>

void hello(void* arg) {
    const char* name = (const char*)arg;
    printf("Hello, %s from lightweight thread!\n", name);
}

int main() {
    // Create a scheduler with 4 worker threads
    lwt_scheduler_t* scheduler = lwt_scheduler_create(4);
    
    // Start the scheduler
    lwt_scheduler_start(scheduler);
    
    // Create a thread
    const char* name = "world";
    lwt_thread_t* thread = lwt_create(scheduler, hello, (void*)name);
    
    // Wait for thread to complete
    lwt_join(thread);
    
    // Clean up
    lwt_scheduler_stop(scheduler);
    lwt_scheduler_destroy(scheduler);
    
    return 0;
}
```

Compile and link with the lwthread library:

```bash
gcc -o hello hello.c -llwthread -lpthread
```

## API Reference

### Scheduler Functions

| Function | Description |
|----------|-------------|
| `lwt_scheduler_t* lwt_scheduler_create(int num_threads)` | Creates a new scheduler with specified number of worker threads |
| `void lwt_scheduler_destroy(lwt_scheduler_t* scheduler)` | Destroys a scheduler and frees its resources |
| `void lwt_scheduler_start(lwt_scheduler_t* scheduler)` | Starts the scheduler and begins executing threads |
| `void lwt_scheduler_stop(lwt_scheduler_t* scheduler)` | Stops the scheduler |

### Thread Functions

| Function | Description |
|----------|-------------|
| `lwt_thread_t* lwt_create(lwt_scheduler_t* scheduler, lwt_func_t func, void* arg)` | Creates a new lightweight thread |
| `void lwt_yield(void)` | Yields execution from current thread to another |
| `void lwt_join(lwt_thread_t* thread)` | Waits for a thread to complete |
| `lwt_thread_t* lwt_current(void)` | Gets the current thread |
| `void lwt_sleep(unsigned int ms)` | Sleeps for the specified duration in milliseconds |

For detailed API documentation, see [docs/api.md](docs/api.md).

## Architecture

LWThread uses an M:N threading model where M user-space threads (lightweight threads) are multiplexed onto N OS threads (worker threads). The architecture consists of the following components:

1. **Scheduler**: Manages worker threads and schedules lightweight threads
2. **Thread Queue**: Maintains the queue of ready-to-run threads
3. **Worker Threads**: OS threads that execute the lightweight threads
4. **Context Switching**: Uses `ucontext.h` for saving and restoring execution state

<p align="center">
  <img src="docs/images/architecture.png" alt="LWThread Architecture" width="600"/>
</p>

## Developer Guide

### Understanding the Code

The library is organized into the following main modules:

- **lwthread.c**: Main implementation and public API
- **thread.c**: Thread implementation and management
- **scheduler.c**: Scheduler and worker thread implementation
- **queue.c**: Thread queue implementation

Each module has a corresponding header file that defines its interface. The public API is exposed through `lwthread.h`.

### Threading Model

LWThread implements an M:N threading model, which means that multiple user-space threads (M) are multiplexed onto fewer OS threads (N). This is achieved through cooperative multitasking, where threads voluntarily yield execution.

The scheduling algorithm is simple:
1. Each OS worker thread runs a loop that fetches threads from the ready queue
2. When a thread yields, it is placed back in the ready queue
3. When a thread blocks (e.g., on join), it is not placed in the ready queue until it is unblocked

This model is similar to Go's goroutines, but with a simpler scheduler.

### Design Patterns

The library uses several design patterns:

- **Opaque Pointers**: The public API uses opaque pointers to hide implementation details
- **Thread-Local Storage**: Uses thread-local storage to track the current thread
- **Producer-Consumer**: The scheduler and worker threads form a producer-consumer pattern
- **Cooperative Multitasking**: Threads cooperatively yield execution

### Extending the Library

#### Adding New Thread Primitives

To add a new thread primitive (e.g., a mutex), you would:

1. Define the struct in an internal header file
2. Implement the primitive in a new source file
3. Add functions to the public API in `lwthread.h`

Example for a simple mutex:

```c
// In a new header file: src/mutex.h
typedef struct {
    lwt_thread_t* owner;
    lwt_thread_queue_t waiting;
} lwt_mutex_t;

// In a new source file: src/mutex.c
lwt_mutex_t* lwt_mutex_create() {
    lwt_mutex_t* mutex = malloc(sizeof(lwt_mutex_t));
    mutex->owner = NULL;
    lwt_queue_init(&mutex->waiting);
    return mutex;
}

void lwt_mutex_lock(lwt_mutex_t* mutex) {
    // Implementation here
}

// Add to public API in lwthread.h
lwt_mutex_t* lwt_mutex_create();
void lwt_mutex_lock(lwt_mutex_t* mutex);
void lwt_mutex_unlock(lwt_mutex_t* mutex);
```

#### Implementing Advanced Scheduling

The current scheduler is simple and uses a FIFO queue. You could extend it to support:

- Priority-based scheduling
- Work stealing
- Deadline scheduling

To implement priority scheduling, you would:

1. Modify the thread structure to include a priority field
2. Replace the simple queue with a priority queue
3. Update the scheduling algorithm to consider priorities

#### Custom Context Switching

The library uses `ucontext.h` for context switching, which is portable but not always the most efficient. For better performance, you could implement custom assembly context switching routines for specific architectures.

## Performance Considerations

### Stack Size

By default, each thread gets a 64KB stack. This is much smaller than OS thread stacks (typically 1-8MB), but still may be larger than needed for simple tasks. Consider customizing the stack size based on your application's needs.

### Context Switching Overhead

Context switches in LWThread are much lighter than OS thread context switches, but still have overhead. Design your application to minimize unnecessary context switches:

- Batch small operations rather than yielding after each one
- Use entity-based concurrency to minimize contention
- Consider the granularity of your threads

### Worker Thread Count

The optimal number of worker threads depends on your hardware and workload:

- For CPU-bound tasks, set worker count equal to the number of CPU cores
- For I/O-bound tasks, you might want more workers than cores
- For mixed workloads, experiment to find the optimal number

## Troubleshooting

### Stack Overflow

If you encounter stack overflow errors, try increasing the stack size:

```c
// Custom stack size (256KB)
lwt_thread_t* thread = lwt_create_with_stack(scheduler, func, arg, 256 * 1024);
```

### Deadlocks

Deadlocks can occur if threads are waiting on each other. Use `lwt_yield()` liberally and ensure proper synchronization.

### Memory Leaks

Remember to free thread resources:
- Call `lwt_thread_free()` after `lwt_join()` if you're managing memory manually
- Always call `lwt_scheduler_destroy()` to clean up scheduler resources

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Appendix: Implementation Details

### Context Switching

LWThread uses the POSIX `ucontext.h` API for context switching. The key functions are:

- `getcontext()`: Saves the current execution context
- `makecontext()`: Modifies a context to call a specified function
- `swapcontext()`: Saves the current context and activates another

Here's a simplified view of the context switching process:

1. Main thread calls `getcontext()` to initialize a new context
2. Main thread calls `makecontext()` to set up the context to execute the thread function
3. When switching threads, the current thread calls `swapcontext()` to save its state and restore another thread's state

### Memory Management

LWThread allocates memory for:

1. Thread structures
2. Thread stacks
3. Scheduler data structures

All memory is managed with standard `malloc()` and `free()`. The library takes care to free all resources when threads complete and when the scheduler is destroyed.

### Thread States

Threads can be in one of the following states:

- `LWT_STATE_NEW`: Thread has been created but not started
- `LWT_STATE_READY`: Thread is ready to run
- `LWT_STATE_RUNNING`: Thread is currently running
- `LWT_STATE_BLOCKED`: Thread is blocked (e.g., on join)
- `LWT_STATE_FINISHED`: Thread has completed execution

### Thread Queue Implementation

The thread queue is implemented as a simple linked list with head and tail pointers for efficient operations. The queue is protected by a mutex to ensure thread safety.