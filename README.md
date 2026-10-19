*This project has been created as part of the 42 curriculum by khelaasr.*

# Codexion

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

## Thread synchronization mechanisms

The project uses POSIX threads (`pthread_create` / `pthread_join`) and
`pthread_mutex_t` only. No condition variable is used: the subject allows
`pthread_cond_t` but does not require it, and the arbitration here is driven by a
priority queue that every waiting coder polls, so a mutex per shared object is
enough. Every shared structure is reachable from a single `t_env` passed to each
thread as its argument, which is why the program needs no global variable.

### The four kinds of mutex

| Mutex | Protects | Held by |
| ----- | -------- | ------- |
| `write_mutex` | standard output | any thread about to log |
| `state_mutex` | `simulation_stop`, `last_compile_start`, `compiles_count` | coders and the monitor |
| `dongle[i].mutex` | one dongle's `is_available` and `last_used_time` | the two coders sharing that dongle |
| `wait_queue.q_mutex` | the scheduling heap | any coder arbitrating for dongles |

### How race conditions are prevented

* **Dongle duplication.** `try_take_dongles()` locks *both* of a coder's dongle
  mutexes before it even tests availability, and only releases them once both
  dongles are marked taken. Testing and taking is therefore a single atomic step,
  so two neighbours can never both conclude that the shared dongle is free.
* **Lost burnout / false burnout.** `last_compile_start` is written by the coder
  under `state_mutex` and read by the monitor under the same mutex, so the monitor
  never reads a half-updated deadline. `report_burnout()` then re-checks the
  deadline a second time while holding the lock before committing, which closes
  the window where a coder starts a new compile between detection and reporting.
* **Interleaved logs.** `ft_log()` builds the whole line into a local buffer and
  emits it with a single `write(2)` while holding `write_mutex`, so two messages
  can never be mixed on one line.
* **Queue corruption.** Every heap operation (`push_queue`, `remove_node`,
  `heapify_up/down`) runs under `q_mutex`.

### Lock ordering

Two rules keep the program free of lock-ordering deadlocks:

1. When both are needed, `write_mutex` is always taken **before** `state_mutex`,
   everywhere in the program.
2. A coder's two dongles are always locked in increasing dongle id, never
   "left then right". Strict left-then-right would form the circular wait
   `1 -> 2 -> ... -> N -> 1` around the table.

`q_mutex` is only ever taken before dongle mutexes, never after.

### Coder / monitor communication

The coders and the monitor communicate exclusively through the fields guarded by
`state_mutex`. A coder publishes progress by updating `last_compile_start` and
`compiles_count`; the monitor consumes those to decide whether somebody burned
out or whether everybody reached `number_of_compiles_required`. In the other
direction the monitor raises the `simulation_stop` flag, which every coder
re-reads under the same mutex at each loop iteration and inside `ft_usleep()`,
so a stopping simulation interrupts sleeping coders promptly instead of waiting
for their current phase to elapse.

## Resources

The project was developed using the following resources:

* `man pthread_create`
* `man pthread_mutex_init`
* `man clock_gettime`
* `man usleep`
* Dining Philosophers Problem
* Coffman's Conditions
* Earliest Deadline First Scheduling

### How AI was used

AI was used to help understand concurrency concepts and review some parts of the code for possible bugs.

The suggestions were reviewed, tested, and understood before being used in the project.