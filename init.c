#include "codexion.h"

/* 
 * Helper function: Converts a string to a long integer.
 * It also checks for negative numbers and invalid characters,
 * returning -1 if any error is found (since all our args must be >= 0).
 */
static long ft_atol(const char *str)
{
    long    res = 0;
    int     i = 0;

    // Skip standard whitespaces
    while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
        i++;
        
    // If it's a negative number, return -1 immediately (not allowed)
    if (str[i] == '-')
        return (-1);
    if (str[i] == '+')
        i++;
        
    // Convert ASCII characters to numbers
    while (str[i] >= '0' && str[i] <= '9')
    {
        res = (res * 10) + (str[i] - '0');
        i++;
    }
    
    // If we stopped but didn't reach the end of the string, it means 
    // there are letters or special characters (e.g., "123a"). Return error.
    if (str[i] != '\0')
        return (-1);
        
    return (res);
}

/* 
 * Parses the command line arguments and fills the t_args structure.
 * Returns 0 on success, 1 on error.
 */
static int parse_args(t_args *args, int argc, char **argv)
{
    // We expect exactly 8 arguments + 1 for the program name (codexion) = 9
    if (argc != 9)
    {
        printf("Error: Invalid number of arguments\n");
        return (1);
    }

    // Convert strings to integers using our strict ft_atol
    args->num_coders = (int)ft_atol(argv[1]);
    args->time_to_burnout = ft_atol(argv[2]);
    args->time_to_compile = ft_atol(argv[3]);
    args->time_to_debug = ft_atol(argv[4]);
    args->time_to_refactor = ft_atol(argv[5]);
    args->num_compiles_required = (int)ft_atol(argv[6]);
    args->dongle_cooldown = ft_atol(argv[7]);

    // Check if any parsing failed (returned -1) or if num_coders is 0
    if (args->num_coders <= 0 || args->time_to_burnout < 0 ||
        args->time_to_compile < 0 || args->time_to_debug < 0 ||
        args->time_to_refactor < 0 || args->num_compiles_required < 0 ||
        args->dongle_cooldown < 0)
    {
        printf("Error: Arguments must be positive integers\n");
        return (1);
    }

    // Strictly check the scheduler string
    if (strcmp(argv[8], "fifo") == 0)
        args->scheduler = FIFO;
    else if (strcmp(argv[8], "edf") == 0)
        args->scheduler = EDF;
    else
    {
        printf("Error: Scheduler must be exactly 'fifo' or 'edf'\n");
        return (1);
    }

    return (0);
}

/* 
 * Initializes the entire environment: mutexes, coders, and dongles.
 */
int init_env(t_env *env, int argc, char **argv)
{
    int i;

    // 1. Parse arguments first
    if (parse_args(&(env->args), argc, argv) != 0)
        return (1);

    // 2. Initialize generic states
    env->simulation_stop = 0;
    env->start_time = 0; // We will set this right before creating threads

    // 3. Initialize the global Mutexes
    if (pthread_mutex_init(&(env->write_mutex), NULL) != 0)
        return (1);
    if (pthread_mutex_init(&(env->state_mutex), NULL) != 0)
        return (1);

    // 4. Allocate memory for coders and dongles arrays
    env->coders = malloc(sizeof(t_coder) * env->args.num_coders);
    env->dongles = malloc(sizeof(t_dongle) * env->args.num_coders);
    if (!env->coders || !env->dongles)
        return (1); // If malloc fails, return error

    // 5. Initialize each Dongle
    for (i = 0; i < env->args.num_coders; i++)
    {
        env->dongles[i].id = i + 1;
        env->dongles[i].is_available = 1;
        env->dongles[i].last_used_time = 0;
        if (pthread_mutex_init(&(env->dongles[i].mutex), NULL) != 0)
            return (1);
    }

    // 6. Initialize each Coder and assign their left and right dongles
    for (i = 0; i < env->args.num_coders; i++)
    {
        env->coders[i].id = i + 1;
        env->coders[i].compiles_count = 0;
        env->coders[i].last_compile_start = 0;
        env->coders[i].env = env;

        // Pointer assignment for the Circular Table
        env->coders[i].left_dongle = &(env->dongles[i]);
        
        // If it's the last coder, their right dongle is the first dongle (id 1)
        if (i == (env->args.num_coders - 1))
            env->coders[i].right_dongle = &(env->dongles[0]);
        else
            env->coders[i].right_dongle = &(env->dongles[i + 1]);
    }

    return (0);
}