/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_take.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khelaasr <khelaasr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 18:34:20 by khelaasr          #+#    #+#             */
/*   Updated: 2026/09/14 18:34:24 by khelaasr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	order_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	*first = coder->left_dongle;
	*second = coder->right_dongle;
	if ((*first)->id > (*second)->id)
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

static int	dongle_ready(t_dongle *d, long now, long cooldown)
{
	if (!d->is_available)
		return (0);
	if ((now - d->last_used_time) < cooldown)
		return (0);
	return (1);
}

/*
** Always locks the two dongles in a fixed global order (lowest id
** first), never "left then right". The last coder's right dongle
** wraps around to id 1, which is LOWER than its own left dongle's id:
** locking strictly left-then-right for every coder would create a
** circular wait across the whole table (1->2->3->...->N->1), a
** textbook Coffman-conditions deadlock. Ordering by id breaks it.
** Both dongles are checked and taken as a single atomic step (never
** holding just one while waiting on the other), which is what
** prevents the classic hold-and-wait deadlock too.
*/
int	try_take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long		now;
	long		cooldown;
	int			taken;

	order_dongles(coder, &first, &second);
	pthread_mutex_lock(&(first->mutex));
	pthread_mutex_lock(&(second->mutex));
	now = get_time_ms();
	cooldown = coder->env->args.dongle_cooldown;
	taken = 0;
	if (dongle_ready(coder->left_dongle, now, cooldown)
		&& dongle_ready(coder->right_dongle, now, cooldown))
	{
		coder->left_dongle->is_available = 0;
		coder->right_dongle->is_available = 0;
		remove_node(coder->env->wait_queue, coder->id);
		taken = 1;
	}
	pthread_mutex_unlock(&(second->mutex));
	pthread_mutex_unlock(&(first->mutex));
	return (taken);
}
