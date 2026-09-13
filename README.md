*This project has been created as part of the 42 curriculum by khelaasr.*

# Codexion

## Description

Codexion simulates coders sharing a limited pool of USB dongles around a
circular table. Each coder is a thread that cycles through three phases:
**compiling** (needs two dongles at once, one "left" and one "right"),
**debugging**, and **refactoring**. There are as many dongles as coders, and
each coder shares one dongle with each of its two neighbours.

The goal is to arbitrate access to the shared dongles correctly: no
deadlocks, no starvation, respect for the chosen scheduling policy (`fifo`
or `edf`), a cooldown period after each dongle is released, and a strict
per-coder deadline — if a coder doesn't start a new compile within
`time_to_burnout` ms of its previous one, it burns out and the simulation
stops. A separate monitor thread detects that and reports it within 10ms.

## Instructions

### Compilation

```sh
make
```

Produces the `codexion` binary. Also supports `all`, `clean`, `fclean`, `re`.

### Execution

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
	time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                     | Meaning                                                      |
|-------------------------------|-----------------------------------------------------------|
| `number_of_coders`            | Number of coder threads (and dongles).                     |
| `time_to_burnout`              | ms since the last compile start before a coder burns out.  |
| `time_to_compile`              | ms spent holding both dongles while compiling.              |
| `time_to_debug`                | ms spent debugging.                                         |
| `time_to_refactor`             | ms spent refactoring.                                        |
| `number_of_compiles_required`  | Stop once every coder reaches this many compiles.           |
| `dongle_cooldown`              | ms a dongle stays unusable after being released.             |
| `scheduler`                    | `fifo` or `edf`.                                              |

All arguments are mandatory, non-negative integers (except `scheduler`).
Invalid input (negative numbers, non-integers, wrong argument count, or an
unknown scheduler) is rejected before the simulation starts.

Example:

```sh
./codexion 5 2000 200 200 200 10 0 fifo
```

## Resources

- `man pthread_create`, `man pthread_mutex_init` — POSIX threading API.
- Wikipedia, "Dining philosophers problem" and "Coffman conditions" — the
  four conditions for deadlock, used to reason about the arbitration logic.
- Wikipedia, "Earliest deadline first scheduling" — reference for `edf`.
- `man clock_gettime`, `man usleep` — timing primitives used here.

**How AI was used:** An AI assistant (Claude) was used to check a few
functions for bugs (it caught a lock-ordering issue between the dongle
mutexes and a couple of deadlock-prone spots) and to ask questions about
concurrency concepts (Coffman's conditions, EDF scheduling) while working
through the design. Everything suggested was reviewed and tested before
being kept, and is understood well enough to explain and modify live during
defence.

## Blocking cases handled

- **Deadlock prevention.** The two dongles a coder needs are always locked
  in a fixed order (lowest id first), never "left then right" — otherwise
  the last coder's wrap-around dongle creates a circular wait around the
  table. Both dongles are also checked and taken together, as one atomic
  step, so a coder never holds one while waiting on the other.
- **Single-coder case.** With only one coder, its left and right dongle are
  the same physical dongle, so it can never gather two — it is meant to
  burn out, which matches the subject's own reference case.
- **Starvation prevention.** Each dongle is granted according to the chosen
  scheduler (arrival order for `fifo`, deadline order for `edf`), with a
  sequence number as a tie-breaker on exact ties. Arbitration is per-dongle,
  so coders that aren't competing for the same dongle can proceed in
  parallel instead of queueing behind an unrelated request.
- **Cooldown handling.** Each dongle remembers when it was released and
  can't be re-taken until `dongle_cooldown` ms have passed.
- **Precise burnout detection.** A monitor thread polls every 1ms and
  re-confirms a burnout right before reporting it, to avoid a false alarm
  if the coder started compiling again in that same instant.
- **Log serialization.** All output goes through one mutex-protected
  logging function using `write()`, so lines are never interleaved and
  nothing prints after the simulation has stopped.
- **Note on tight parameters:** with 5 coders in a ring, at most 2 can
  physically compile at the same time. Under a large `dongle_cooldown`,
  this can make some `time_to_burnout` values a true zero-slack boundary
  (e.g. burnout exactly equal to the worst-case wait) where the outcome can
  vary slightly with timing. Giving a bit more slack makes it pass reliably.

## Thread synchronization mechanisms

- `write_mutex` serializes all log output.
- `state_mutex` protects `simulation_stop` and each coder's compile
  counters/timestamps, shared between the coder threads and the monitor.
- Each dongle has its own mutex protecting its availability flag and
  release timestamp.
- The wait queue has its own mutex protecting the priority heap used for
  `fifo`/`edf` arbitration.
- `write_mutex` is always locked before `state_mutex`, and the two dongle
  mutexes are always locked lowest-id-first — these two fixed lock orders
  are what keep the whole program free of lock-ordering deadlocks.
- The monitor and the coder threads never signal each other directly: the
  monitor just polls the shared, mutex-protected state, and any thread that
  sees `simulation_stop == 1` unwinds and exits. No condition variables were
  needed since every wait here is short and cheap to poll.