/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khelaasr <khelaasr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 18:35:30 by khelaasr          #+#    #+#             */
/*   Updated: 2026/09/14 18:35:44 by khelaasr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timespec	ts;
	long			time_in_ms;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
	{
		fprintf(stderr, "Error: clock_gettime failed\n");
		return (-1);
	}
	time_in_ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
	return (time_in_ms);
}

void	ft_usleep(long time_to_wait_ms, t_env *env)
{
	long	start_time;
	int		stop;

	start_time = get_time_ms();
	while ((get_time_ms() - start_time) < time_to_wait_ms)
	{
		pthread_mutex_lock(&(env->state_mutex));
		stop = env->simulation_stop;
		pthread_mutex_unlock(&(env->state_mutex));
		if (stop == 1)
			break ;
		usleep(100);
	}
}

/*
** Serializes every log line through write_mutex (outer lock) then
** state_mutex (inner lock), always in that order everywhere in the
** program: that fixed order is what keeps the whole project free of
** lock-ordering deadlocks. write(2) is used directly (rather than
** buffered printf) so the message reaches the terminal immediately,
** which matters for the <=10ms burnout-reporting precision requirement.
** Once the simulation has stopped, regular state messages are
** suppressed so nothing gets printed after the terminating line.
*/
void	ft_log(t_env *env, int id, const char *state)
{
	long	ts;
	char	buf[64];
	int		len;

	pthread_mutex_lock(&(env->write_mutex));
	pthread_mutex_lock(&(env->state_mutex));
	if (env->simulation_stop == 0)
	{
		ts = get_time_ms() - env->start_time;
		len = snprintf(buf, sizeof(buf), "%ld %d %s\n", ts, id, state);
		if (len > 0)
			write(STDOUT_FILENO, buf, (size_t)len);
	}
	pthread_mutex_unlock(&(env->state_mutex));
	pthread_mutex_unlock(&(env->write_mutex));
}
