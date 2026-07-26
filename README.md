# Codexion

<div align="center">
  <img src="readme_gif/output_gif.gif" alt="Codexion multithreaded simulation demo" width="100%" />
</div>

<div align="center">

[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Pthreads](https://img.shields.io/badge/threads-pthreads-4CAF50.svg)](https://man7.org/linux/man-pages/man7/pthreads.7.html)
[![Build](https://img.shields.io/badge/build-make-ff9800.svg)](Makefile)
[![Status](https://img.shields.io/badge/status-simulation-success-brightgreen.svg)](#)

</div>

## ✨ Hero

Codexion is a compact multithreaded simulation written in C that models a team of coders competing for shared dongles while they compile, debug, refactor, and eventually stop once they either meet their target output or burn out.

It is a practical study in concurrency, synchronization, resource ownership, and scheduling under contention.

## 🎬 Demo

The project is best understood by watching the live simulation in action. The embedded GIF above shows the runtime behavior of the threads as they request, acquire, release, and re-enter the shared resource cycle.

## 🧠 About

The implementation is centered around a shared simulation state and a set of coder threads. Each coder is represented by a `t_coder` structure and each shared dongle by a `t_dongle` structure. The program initializes a fixed number of coders and dongles, launches one thread per coder, and uses a monitor loop to decide when the simulation should finish.

The repository does not implement a general-purpose scheduler or a full operating system kernel. It is a focused, concrete demonstration of how synchronization primitives behave in a controlled concurrent environment.

<details>
<summary><strong>What this project is really doing</strong></summary>

It simulates a small concurrency problem in which several workers compete for a small set of shared resources. The interesting part is not the business logic of coding itself, but the way the program coordinates access to those resources without races, deadlocks, or runaway threads.
</details>

## ⚙️ Features

- Multithreaded simulation with one POSIX thread per coder
- Shared resource management through dedicated dongle objects
- Two scheduling modes: FIFO and EDF
- Burnout detection through a monitor loop
- Condition-variable-based waiting for resource availability
- Explicit cleanup of allocated memory and synchronization objects
- Millisecond-precision timing for the simulation clock

## 🏗️ Architecture

The core design is intentionally simple and explicit:

```text
main.c
  └─ parses input and launches the simulation

sim_init.c
  └─ allocates coders, dongles, and synchronization primitives

threads_creat.c
  └─ creates coder threads and joins them at shutdown

coder_routine.c
  └─ executes each coder's lifecycle: request, acquire, work, release

monitor.c
  └─ checks burnout and completion conditions

heap.c
  └─ manages per-dongle request queues for FIFO/EDF ordering

time.c
  └─ provides timing helpers and sleep logic
```

## 🧵 Thread Model

Each coder runs in its own pthread. The lifecycle is straightforward:

1. The thread waits until the simulation start flag is set.
2. It requests access to its two assigned dongles.
3. It acquires them when the resource state allows it.
4. It performs the compile/debug/refactor phases.
5. It releases the dongles and repeats until it reaches the configured compile target or the monitor decides the run is over.

The code also introduces a small stagger by giving odd-numbered coders an initial `usleep(100)` delay, which helps expose contention patterns during execution.

## 🔐 Synchronization

The synchronization strategy is built around the primitives defined in the repository:

- `pthread_mutex_t` protects shared state such as compile counters, last-compile time, simulation completion, and printing.
- `pthread_cond_t` and `pthread_cond_wait` / `pthread_cond_timedwait` coordinate dongle availability.
- `pthread_cond_broadcast` wakes all waiters when a dongle is released.
- A dedicated `print_mutex` serializes terminal output so log lines remain readable.

This is not a lock-free design. It is a conventional, explicit synchronization model built for clarity and correctness.

## 🧰 Resource Management

The simulation uses a fixed set of dongles allocated at startup. Each dongle stores:

- whether it is currently taken
- when it becomes available again
- a small request queue
- its own mutex and condition variable

Each coder is assigned two dongles, one on the left and one on the right. The resource ownership model is therefore local, explicit, and easy to trace in the code.

## 📋 Scheduling Strategy

Codexion supports two scheduling modes selected by the final command-line argument:

| Mode | Behavior |
| --- | --- |
| `fifo` | Requests are serviced in queue order. |
| `edf` | Requests are reordered by a priority value derived from the coder's deadline. |

The EDF priority is computed as:

$$
\text{priority} = \text{last\_compile\_t} + \text{time\_to\_burnout}
$$

The implementation stores request entries in a per-dongle queue and reorders them when EDF mode is enabled.

## 🛡️ Deadlock Prevention

The repository avoids the classic circular-wait pattern by construction. A coder only attempts to acquire the two dongles it has already requested, and the request path is coordinated through per-dongle queues and availability checks. The code also ensures that a dongle is released before the next waiting cycle begins, reducing the chance of a thread holding one resource while waiting indefinitely on another.

## 🚦 Starvation Prevention

Starvation risk is reduced in two ways:

- FIFO mode preserves request order.
- EDF mode reorders requests by the earliest deadline.
- When a dongle is released, waiters are woken with `pthread_cond_broadcast`, allowing the next eligible request to proceed.

The design is not a formal starvation-proof scheduler, but it does incorporate queue ordering and wake-up signaling to keep contention moving forward.

## 📁 Project Structure

| File | Purpose |
| --- | --- |
| [main.c](main.c) | Parses arguments, starts the simulation, and joins threads. |
| [codexion.h](codexion.h) | Shared structures and function declarations. |
| [data_init.c](data_init.c) | Validates and converts CLI input into simulation parameters. |
| [sim_init.c](sim_init.c) | Allocates and initializes coders, dongles, and mutexes. |
| [threads_creat.c](threads_creat.c) | Creates pthreads and cleans up on failure. |
| [coder_routine.c](coder_routine.c) | Implements each coder's main loop. |
| [coder_routine_utils.c](coder_routine_utils.c) | Handles logging, startup synchronization, compile updates, and requests. |
| [heap.c](heap.c) | Implements the request queue and EDF ordering logic. |
| [monitor.c](monitor.c) | Watches burnout and completion conditions. |
| [time.c](time.c) | Provides timing and sleeping utilities. |
| [Makefile](Makefile) | Builds the executable with pthread support. |

## ▶️ Installation

Build the binary with:

```bash
make
```

The build produces an executable named `codexion`.

## 🚀 Usage

Run the simulator with the following arguments:

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles> <dongle_cooldown> <fifo|edf>
```

### Example

```bash
./codexion 5 800 200 100 100 5 50 fifo
```

This example launches five coders, gives them a burnout window of 800 ms, and uses FIFO ordering for dongle access.

## ⚡ Performance

The implementation is intentionally lightweight and readable rather than highly optimized. It uses a small bounded queue per dongle, fixed-size arrays for the simulation objects, and a polling monitor loop with a 1 ms delay. The overhead is dominated by mutex locking, condition-variable wakeups, and short sleep intervals between simulation phases.

## 🧭 Technical Decisions

Several design choices are visible in the code and are worth highlighting:

- The simulation uses a single shared `t_sim` object to hold global state.
- Each dongle has its own mutex and condition variable to avoid unnecessary contention.
- Logging is serialized with a dedicated mutex to keep output coherent.
- The monitor is implemented as a cooperative loop rather than a separate pthread.
- The request queue is intentionally small and fixed-size, matching the structure of the simulation.

## 🧩 Challenges

The project is small, but the implementation still has to solve several real concurrency problems:

- preventing races on compile count and last-compile timestamps
- ensuring waiters wake up correctly when resources become available again
- coordinating release timing with cooldown behavior
- stopping the simulation cleanly when burnout or completion conditions are met

Those challenges are handled explicitly through mutexes, condition variables, and carefully placed state checks.

## 🌱 Future Improvements

The current codebase is already functional, but a few natural next steps would be:

- richer runtime statistics for queue waiting and turnaround time
- a more detailed trace format for debugging contention
- configurable queue depth and richer scheduling metrics

## 👤 Authors

The repository headers identify the implementation as belonging to nel-adao.
