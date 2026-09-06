#ifndef CODEXION_H
# define CODEXION_H

# define _POSIX_C_SOURCE 199309L // Enables POSIX clock features

# include <pthread.h>
# include <time.h>      /* Added for clock_gettime and CLOCK_MONOTONIC */
# include <sys/time.h>  /* Kept just in case we need generic time structs */
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>

/* Enum to define which scheduler we are using */
typedef enum e_scheduler {
    FIFO,
    EDF
} t_scheduler;

/* Structure to hold all the command line arguments */
typedef struct s_args {
    int             num_coders;
    long            time_to_burnout;
    long            time_to_compile;
    long            time_to_debug;
    long            time_to_refactor;
    int             num_compiles_required;
    long            dongle_cooldown;
    t_scheduler     scheduler;
} t_args;

/* Forward declarations */
typedef struct s_env t_env;
typedef struct s_coder t_coder;

/* Structure for the Dongle (Resource) */
typedef struct s_dongle {
    int             id;
    pthread_mutex_t mutex;
    long            last_used_time; // To calculate the cooldown
    int             is_available;   // 1 if on the table, 0 if taken
} t_dongle;

/* Node for the Priority Queue (Heap) for Schedulers */
typedef struct s_qnode {
    t_coder         *coder;
    long            priority;       // Will be arrival_time (FIFO) or deadline (EDF)
} t_qnode;

/* Priority Queue (Min-Heap array representation) */
typedef struct s_queue {
    t_qnode         *array;
    int             size;
    int             capacity;
    pthread_mutex_t q_mutex;        // To protect queue operations
} t_queue;

/* Structure representing a single coder */
struct s_coder {
    int             id;
    pthread_t       thread_id;
    long            last_compile_start;
    int             compiles_count;
    t_dongle        *left_dongle;   // Pointer to left dongle
    t_dongle        *right_dongle;  // Pointer to right dongle
    t_env           *env;
};

/* Structure for the main environment holding everything */
struct s_env {
    t_args          args;
    t_coder         *coders;
    t_dongle        *dongles;       // Array of all dongles
    t_queue         *wait_queue;    // Queue for coders waiting for dongles
    pthread_mutex_t write_mutex;
    pthread_mutex_t state_mutex;
    long            start_time;
    int             simulation_stop;
};

#endif