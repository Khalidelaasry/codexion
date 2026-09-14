/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khelaasr <khelaasr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 18:35:18 by khelaasr          #+#    #+#             */
/*   Updated: 2026/09/14 18:35:20 by khelaasr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Re-checks (under the correct write_mutex -> state_mutex lock order
** used everywhere else in the program) that the given coder is STILL
** past its burnout deadline before committing to the burnout report.
** This closes a race window: the coder could have started a new
** compile (updating last_compile_start) in the moment between the
** initial detection in monitor_routine() and this confirmation, which
** would make it a false alarm. Only if it is still confirmed do we
** flip simulation_stop and print, atomically, so no other thread can
** sneak a log line in after it. Returns 1 if a burnout was reported.
*/
static int	report_burnout(t_env *env, int coder_id)
{
	t_coder	*c;
	long	now;
	char	buf[64];
	int		len;
	int		reported;

	c = &(env->coders[coder_id - 1]);
	reported = 0;
	pthread_mutex_lock(&(env->write_mutex));
	pthread_mutex_lock(&(env->state_mutex));
	now = get_time_ms();
	if (env->simulation_stop == 0
		&& (now - c->last_compile_start) >= env->args.time_to_burnout)
	{
		env->simulation_stop = 1;
		len = snprintf(buf, sizeof(buf), "%ld %d burned out\n",
				now - env->start_time, coder_id);
		if (len > 0)
			write(STDOUT_FILENO, buf, (size_t)len);
		reported = 1;
	}
	pthread_mutex_unlock(&(env->state_mutex));
	pthread_mutex_unlock(&(env->write_mutex));
	return (reported);
}

/*
** Scans every coder once, under state_mutex, and reports whether any
** of them is currently past its burnout deadline (0 means none) and
** whether every coder has already reached num_compiles_required.
*/
static int	scan_coders(t_env *env, int *all_done)
{
	int		i;
	long	now;
	int		burnout_id;

	now = get_time_ms();
	*all_done = 1;
	burnout_id = 0;
	i = 0;
	while (i < env->args.num_coders)
	{
		if (env->coders[i].compiles_count < env->args.num_compiles_required)
			*all_done = 0;
		if (burnout_id == 0 && (now - env->coders[i].last_compile_start)
			>= env->args.time_to_burnout)
			burnout_id = env->coders[i].id;
		i++;
	}
	return (burnout_id);
}

/*
** The monitor thread: polls every coder's deadline
** (last_compile_start + time_to_burnout) at a fine enough interval to
** guarantee the burnout message is printed within 10ms of the real
** deadline, and also watches for the normal end condition (every
** coder reached num_compiles_required), at which point it simply
** exits and lets the coder threads wind down and get joined by
** main().
*/
void	*monitor_routine(void *arg)
{
	t_env	*env;
	int		all_done;
	int		burnout_id;
	int		stop;

	env = (t_env *)arg;
	while (1)
	{
		usleep(1000);
		pthread_mutex_lock(&(env->state_mutex));
		stop = env->simulation_stop;
		pthread_mutex_unlock(&(env->state_mutex));
		if (stop == 1)
			return (NULL);
		pthread_mutex_lock(&(env->state_mutex));
		burnout_id = scan_coders(env, &all_done);
		pthread_mutex_unlock(&(env->state_mutex));
		if (burnout_id != 0 && report_burnout(env, burnout_id))
			return (NULL);
		if (burnout_id == 0 && all_done)
			return (NULL);
	}
}
