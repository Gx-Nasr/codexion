# 🚀 Codexion

> A multithreaded coding simulation written in C using POSIX Threads, Mutexes, and Condition Variables.

Codexion is a synchronization project inspired by the classic Dining Philosophers problem.  
Each **Coder** is represented by a thread competing for shared **Dongles** (resources) in order to compile code before reaching burnout.

The project focuses on concurrent programming, synchronization primitives, scheduling algorithms, and race-condition prevention.

---

## ✨ Features

- 🧵 Multithreaded simulation using POSIX Threads.
- 🔒 Safe synchronization using Mutexes.
- 📢 Condition Variables for efficient waiting.
- ⚡ FIFO and EDF scheduling modes.
- ⏱️ Millisecond-precision timing.
- 💀 Burnout detection by a dedicated monitor.
- 🧠 Deadlock-free resource acquisition.
- 🧹 Proper cleanup of allocated memory and synchronization objects.

---

## 🏗️ Architecture

```
                +----------------+
                |    Monitor     |
                +----------------+
                       |
        ---------------------------------
        |               |               |
    Coder 1         Coder 2         Coder N
        |               |               |
      Dongles ------ Shared Resources ------
```

---

## ⚙️ Build

```bash
make
```

Run:

```bash
./codexion \
<number_of_coders> \
<time_to_burnout> \
<time_to_compile> \
<time_to_debug> \
<time_to_refactor> \
<number_of_compiles> \
<dongle_cooldown> \
<fifo|edf>
```

Example:

```bash
./codexion 5 800 200 100 100 5 50 fifo
```

---

## 🧵 Synchronization

The project uses:

- `pthread_create`
- `pthread_join`
- `pthread_mutex_init`
- `pthread_mutex_lock`
- `pthread_mutex_unlock`
- `pthread_mutex_destroy`
- `pthread_cond_init`
- `pthread_cond_wait`
- `pthread_cond_timedwait`
- `pthread_cond_broadcast`
- `pthread_cond_destroy`

All shared data is protected with mutexes to prevent data races.

---

## 📋 Scheduling

### FIFO

Requests are handled in the order they arrive.

```
Coder 1
Coder 2
Coder 3
```

---

### EDF (Earliest Deadline First)

Priority is calculated from each coder's remaining time before burnout.

The coder with the earliest deadline receives the dongle first.

---

## 📂 Project Structure

```
.
├── main.c
├── parser.c
├── init.c
├── monitor.c
├── routine.c
├── logger.c
├── queue.c
├── utils.c
├── cleanup.c
├── codexion.h
└── Makefile
```

---

## 🛡️ Thread Safety

The simulation protects:

- compile count
- last compile time
- simulation state
- start flag
- printing
- dongle ownership
- request queues

using mutexes and condition variables.

---

## 📊 Simulation Flow

```
Create Threads
      │
      ▼
Wait For Start Signal
      │
      ▼
Request Dongles
      │
      ▼
Acquire Dongles
      │
      ▼
Compile
      │
      ▼
Debug
      │
      ▼
Refactor
      │
      ▼
Release Dongles
      │
      ▼
Repeat
```

---

## 🎯 Learning Objectives

This project demonstrates:

- Concurrent Programming
- POSIX Threads
- Mutex Synchronization
- Condition Variables
- Race Condition Prevention
- Deadlock Avoidance
- Scheduling Algorithms
- Resource Management
- Memory Management
- Thread Lifecycle

---

## 🛠️ Technologies

- C
- POSIX Threads
- Mutex
- Condition Variables
- Makefile
- Linux

---

## 📄 License

This project was developed for educational purposes at **1337 / 42 Network**.

---

## 👤 Author

**Gx-Nasr**

GitHub: https://github.com/Gx-NAsr