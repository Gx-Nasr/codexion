*This project has been created as part of the 42 curriculum by nel-adao.*

# Codexion

## Description

Codexion is a multithreaded synchronization project inspired by the classic Dining Philosophers problem. The goal is to simulate multiple coders competing for shared resources (dongles) while preventing common concurrency issues such as deadlocks, starvation, and race conditions.

The project supports two scheduling policies:

* **FIFO (First In, First Out)**
* **EDF (Earliest Deadline First)**

A custom priority queue (heap) is used to manage access to shared resources according to the selected scheduling algorithm.

---

## Instructions

### Compile

```bash
make
```

### Run

```bash
./codexion number_of_coders time_to_burnout time_to_compile \
time_to_debug time_to_refactor number_of_compiles \
dongle_cooldown fifo|edf
```

### Example

```bash
./codexion 5 600 200 200 100 2 0 edf
```

Arguments:

* `number_of_coders` – Number of coder threads.
* `time_to_burnout` – Maximum time without compiling.
* `time_to_compile` – Compilation duration.
* `time_to_debug` – Debugging duration.
* `time_to_refactor` – Refactoring duration.
* `number_of_compiles` – Required compile count before stopping.
* `dongle_cooldown` – Cooldown after releasing a dongle.
* `fifo | edf` – Scheduling policy.

---

## Blocking Cases Handled

The implementation handles the following concurrency issues:

* Deadlock prevention by alternating the order in which coders acquire dongles.
* Starvation prevention using FIFO or EDF scheduling.
* Cooldown handling after releasing a dongle before it becomes available again.
* Precise burnout detection through a dedicated monitor thread.
* Serialized logging to avoid mixed or corrupted output.
* Safe program termination by waking waiting threads when the simulation ends.

---

## Thread Synchronization Mechanisms

The project uses the following synchronization primitives:

* **pthread_mutex_t**

  * Protects shared data such as coder state, dongles, logging, and simulation status.
  * Prevents race conditions while reading or modifying shared resources.

* **pthread_cond_t**

  * Allows coders to wait until a dongle becomes available.
  * Used together with timed waits to support cooldown handling.

* **Monitor Thread**

  * Continuously checks burnout conditions and compile limits.
  * Stops the simulation safely and notifies all waiting threads.

These mechanisms ensure thread-safe communication between coders and the monitor while coordinating access to shared resources.

---

## Resources

### Documentation

* POSIX Threads (pthreads)
* Linux `pthread` Manual Pages
* Valgrind (Memcheck & Helgrind)
* The Dining Philosophers Problem
* 42 Subject Documentation

### AI Usage

AI was used primarily as a learning aid to better understand multithreading concepts, synchronization mechanisms, and concurrency theory. It was also used to suggest edge cases for testing and validation. The implementation, debugging, design decisions, and final code were developed and completed manually.
