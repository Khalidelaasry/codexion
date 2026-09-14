/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                         :+:      :+:    :+:     */
/*                                                     +:+ +:+         +:+    */
/*   By: khelaasr <khelaasr@student.1337.ma>         +#+  +:+       +#+      */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by khelaasr #+#    #+#                     */
/*   Updated: 2026/09/12 00:00:00 by khelaasr ###   ########.fr              */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** A coder polls the wait queue until it is the highest-priority
** request AND both of its dongles are simultaneously free and past
** cooldown (see attempt_acquire() / try_take_dongles()).
*/
static void	acquire_dongles(t_coder *coder)
{
	int	pushed;
	int	stop;

	pushed = 0;
	while (1)
	{
		pthread_mutex_lock(&(coder->env->state_mutex));
		stop = coder->env->simulation_stop;
		pthread_mutex_unlock(&(coder->env->state_mutex));
		if (stop == 1)
			return ;
		if (attempt_acquire(coder, &pushed))
			break ;
		usleep(100);
	}
	ft_log(coder->env, coder->id, "has taken a dongle");
	ft_log(coder->env, coder->id, "has taken a dongle");
}

/*
** Returns both dongles to the table and starts their cooldown timer.
*/
static void	release_dongles(t_coder *coder)
{
	long	current_time;

	current_time = get_time_ms();
	pthread_mutex_lock(&(coder->left_dongle->mutex));
	coder->left_dongle->is_available = 1;
	coder->left_dongle->last_used_time = current_time;
	pthread_mutex_unlock(&(coder->left_dongle->mutex));
	pthread_mutex_lock(&(coder->right_dongle->mutex));
	coder->right_dongle->is_available = 1;
	coder->right_dongle->last_used_time = current_time;
	pthread_mutex_unlock(&(coder->right_dongle->mutex));
}

/*
** Marks the start of a compile: this is the timestamp the monitor
** thread watches to detect burnout. Returns 1 if the simulation was
** already flagged to stop while we were waiting for dongles.
*/
static int	start_compile(t_coder *coder)
{
	pthread_mutex_lock(&(coder->env->state_mutex));
	if (coder->env->simulation_stop == 1)
	{
		pthread_mutex_unlock(&(coder->env->state_mutex));
		return (1);
	}
	coder->last_compile_start = get_time_ms();
	coder->compiles_count++;
	pthread_mutex_unlock(&(coder->env->state_mutex));
	return (0);
}

static void	debug_and_refactor(t_coder *coder)
{
	ft_log(coder->env, coder->id, "is debugging");
	ft_usleep(coder->env->args.time_to_debug, coder->env);
	ft_log(coder->env, coder->id, "is refactoring");
	ft_usleep(coder->env->args.time_to_refactor, coder->env);
}

/*
** The main lifecycle loop of a Coder thread: request dongles, compile,
** release them, debug, refactor, repeat - until either the required
** number of compiles is reached or the simulation is stopped (by a
** burnout, its own or anyone else's).
*/
void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		stop;

	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		usleep(500);
	while (1)
	{
		pthread_mutex_lock(&(coder->env->state_mutex));
		stop = coder->env->simulation_stop;
		pthread_mutex_unlock(&(coder->env->state_mutex));
		if (stop == 1)
			break ;
		acquire_dongles(coder);
		if (start_compile(coder) == 1)
			break ;
		ft_log(coder->env, coder->id, "is compiling");
		ft_usleep(coder->env->args.time_to_compile, coder->env);
		release_dongles(coder);
		if (coder->compiles_count >= coder->env->args.num_compiles_required)
			break ;
		debug_and_refactor(coder);
	}
	return (NULL);
}
