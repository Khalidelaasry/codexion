# Codexion

*This project has been created as part of the 42 curriculum by khelaasr.*

## Description

Codexion is a multithreading project written in C.

The project simulates coders sharing a limited number of USB dongles around a circular table.

Each coder is a thread that repeatedly goes through three phases:

* **Compiling** — requires two dongles.
* **Debugging**
* **Refactoring**

The goal is to manage the shared dongles correctly while avoiding deadlocks and starvation.

The program supports two scheduling policies:

* `fifo`
* `edf`

It also handles dongle cooldowns and coder burnout deadlines.

## Instructions

### Compilation

Compile the project with:

```bash
make
```

Other available commands:

```bash
make clean
make fclean
make re
```

This produces the `codexion` executable.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

### Arguments

| Argument                      | Description                                          |
| ----------------------------- | ---------------------------------------------------- |
| `number_of_coders`            | Number of coder threads and dongles                  |
| `time_to_burnout`             | Maximum time before a coder burns out                |
| `time_to_compile`             | Time spent compiling                                 |
| `time_to_debug`               | Time spent debugging                                 |
| `time_to_refactor`            | Time spent refactoring                               |
| `number_of_compiles_required` | Number of compilations required                      |
| `dongle_cooldown`             | Time a dongle stays unavailable after being released |
| `scheduler`                   | Scheduling policy: `fifo` or `edf`                   |

All times are given in milliseconds.

### Example

```bash
./codexion 5 2000 200 200 200 10 0 fifo
```

## Scheduling

### FIFO

`fifo` means **First In, First Out**.

Coders are served according to the order in which they request the dongles.

### EDF

`edf` means **Earliest Deadline First**.

The coder with the closest burnout deadline gets priority.

## Blocking Cases Handled

### Deadlock Prevention

A coder needs two dongles to compile.

The two dongles are always acquired in a fixed order based on their IDs. This prevents circular waiting between coders.

### Single Coder

When there is only one coder, both sides refer to the same dongle.

The coder cannot get two different dongles, so it eventually burns out.

### Starvation Prevention

The waiting queue uses the selected scheduling policy to decide which coder gets priority.

A sequence number is used when two requests have the same priority.

### Dongle Cooldown

After a dongle is released, it cannot immediately be used again.

The dongle becomes available only after the configured cooldown time.

### Burnout Detection

A monitor thread checks the coders regularly.

If a coder does not start a new compilation before `time_to_burnout`, the coder burns out and the simulation stops.

### Safe Logging

All output is protected by a mutex so that different threads cannot write over each other.

## Thread Synchronization

The project uses POSIX threads and mutexes.

Main synchronization mechanisms include:

* `pthread_create()` for creating coder and monitor threads.
* `pthread_mutex` for protecting shared data.
* A mutex for the simulation state.
* A mutex for each dongle.
* A mutex for the waiting queue.
* A mutex for logging.

The program also follows fixed lock orders to avoid lock-ordering problems and deadlocks.

## Resources

The project was developed using the following resources:

* `man pthread_create`
* `man pthread_mutex_init`
* `man clock_gettime`
* `man usleep`
* Dining Philosophers Problem
* Coffman's Conditions
* Earliest Deadline First Scheduling

## AI Usage

AI was used to help understand concurrency concepts and review some parts of the code for possible bugs.

The suggestions were reviewed, tested, and understood before being used in the project.