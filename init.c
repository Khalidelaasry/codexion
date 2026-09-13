/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                              :+:      :+:    :+:  */
/*                                                     +:+ +:+         +:+    */
/*   By: YOUR_LOGIN <YOUR_LOGIN@student.1337.ma>     +#+  +:+       +#+       */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by YOUR_LOGIN        #+#    #+#             */
/*   Updated: 2026/09/12 00:00:00 by YOUR_LOGIN       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_env *env)
{
	int	i;

	env->dongles = malloc(sizeof(t_dongle) * env->args.num_coders);
	if (!env->dongles)
		return (1);
	i = 0;
	while (i < env->args.num_coders)
	{
		env->dongles[i].id = i + 1;
		env->dongles[i].is_available = 1;
		env->dongles[i].last_used_time = 0;
		if (pthread_mutex_init(&(env->dongles[i].mutex), NULL) != 0)
			return (1);
		i++;
	}
	return (0);
}

/*
** Assigns each coder its left and right dongle around the circular
** table. The last coder's right dongle wraps around to dongle id 1.
*/
static void	init_coders(t_env *env)
{
	int	i;

	i = 0;
	while (i < env->args.num_coders)
	{
		env->coders[i].id = i + 1;
		env->coders[i].compiles_count = 0;
		env->coders[i].last_compile_start = 0;
		env->coders[i].env = env;
		env->coders[i].left_dongle = &(env->dongles[i]);
		if (i == (env->args.num_coders - 1))
			env->coders[i].right_dongle = &(env->dongles[0]);
		else
			env->coders[i].right_dongle = &(env->dongles[i + 1]);
		i++;
	}
}

/*
** Initializes the entire environment: mutexes, coders, dongles and the
** scheduling queue. Zeroes the struct first so that a partially
** initialized env is always safe to hand to free_all() on any early
** failure return below.
*/
int	init_env(t_env *env, int argc, char **argv)
{
	memset(env, 0, sizeof(t_env));
	if (parse_args(&(env->args), argc, argv) != 0)
		return (1);
	env->simulation_stop = 0;
	env->start_time = 0;
	if (pthread_mutex_init(&(env->write_mutex), NULL) != 0)
		return (1);
	if (pthread_mutex_init(&(env->state_mutex), NULL) != 0)
		return (1);
	env->coders = malloc(sizeof(t_coder) * env->args.num_coders);
	if (!env->coders)
		return (1);
	if (init_dongles(env) != 0)
		return (1);
	init_coders(env);
	if (init_queue(env) != 0)
		return (1);
	return (0);
}
