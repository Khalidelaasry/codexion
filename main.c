/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                          :+:      :+:    :+:     */
/*                                                     +:+ +:+         +:+    */
/*   By: khelaasr <khelaasr@student.1337.ma>         +#+  +:+       +#+      */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by khelaasr #+#    #+#                     */
/*   Updated: 2026/09/12 00:00:00 by khelaasr ###   ########.fr              */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Cleans up all allocated resources and destroys mutexes. Safe to
** call on a partially-initialized env (everything starts zeroed by
** init_env's memset).
*/
void	free_all(t_env *env)
{
	int	i;

	if (env->dongles)
	{
		i = 0;
		while (i < env->args.num_coders)
		{
			pthread_mutex_destroy(&(env->dongles[i].mutex));
			i++;
		}
		free(env->dongles);
	}
	if (env->coders)
		free(env->coders);
	if (env->wait_queue)
	{
		if (env->wait_queue->array)
			free(env->wait_queue->array);
		pthread_mutex_destroy(&(env->wait_queue->q_mutex));
		free(env->wait_queue);
	}
	pthread_mutex_destroy(&(env->write_mutex));
	pthread_mutex_destroy(&(env->state_mutex));
}

/*
** Spawns every coder thread. On failure partway through, flags
** simulation_stop and joins exactly the threads that were actually
** created (never touches an uninitialized pthread_t).
*/
static int	spawn_coders(t_env *env)
{
	int	i;
	int	created;

	created = 0;
	while (created < env->args.num_coders)
	{
		env->coders[created].last_compile_start = env->start_time;
		if (pthread_create(&(env->coders[created].thread_id), NULL,
				&coder_routine, &(env->coders[created])) != 0)
			break ;
		created++;
	}
	if (created == env->args.num_coders)
		return (0);
	pthread_mutex_lock(&(env->state_mutex));
	env->simulation_stop = 1;
	pthread_mutex_unlock(&(env->state_mutex));
	i = 0;
	while (i < created)
	{
		pthread_join(env->coders[i].thread_id, NULL);
		i++;
	}
	return (1);
}

static int	join_all(t_env *env, pthread_t monitor_thread)
{
	int	i;

	i = 0;
	while (i < env->args.num_coders)
	{
		if (pthread_join(env->coders[i].thread_id, NULL) != 0)
			return (1);
		i++;
	}
	if (pthread_join(monitor_thread, NULL) != 0)
		return (1);
	return (0);
}

/*
** Starts the simulation: creates all coder threads and the monitor
** thread, then waits for all of them to finish.
*/
static int	start_simulation(t_env *env)
{
	pthread_t	monitor_thread;
	int			i;

	env->start_time = get_time_ms();
	if (spawn_coders(env) != 0)
	{
		fprintf(stderr, "Error: Failed to create coder thread\n");
		return (1);
	}
	if (pthread_create(&monitor_thread, NULL, &monitor_routine, env) != 0)
	{
		fprintf(stderr, "Error: Failed to create monitor thread\n");
		pthread_mutex_lock(&(env->state_mutex));
		env->simulation_stop = 1;
		pthread_mutex_unlock(&(env->state_mutex));
		i = 0;
		while (i < env->args.num_coders)
		{
			pthread_join(env->coders[i].thread_id, NULL);
			i++;
		}
		return (1);
	}
	return (join_all(env, monitor_thread));
}

int	main(int argc, char **argv)
{
	t_env	env;

	if (init_env(&env, argc, argv) != 0)
	{
		free_all(&env);
		return (1);
	}
	if (start_simulation(&env) != 0)
	{
		free_all(&env);
		return (1);
	}
	free_all(&env);
	return (0);
}
