#include "codexion.h"

long get_time_ms(void)
{
    struct timespec ts;
    long            time_in_ms;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
    {
        printf("Error: clock_gettime failed\n");
        return (-1);
    }

    time_in_ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

    return (time_in_ms);
}

void ft_usleep(long time_to_wait_ms, t_env *env)
{
    long start_time;

    start_time = get_time_ms();

    while ((get_time_ms() - start_time) < time_to_wait_ms)
    {
        pthread_mutex_lock(&(env->state_mutex));

        if (env->simulation_stop == 1)
        {
            pthread_mutex_unlock(&(env->state_mutex));
            break;
        }
        pthread_mutex_unlock(&(env->state_mutex));

        usleep(50);
    }
}