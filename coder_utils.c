/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                       :+:      :+:    :+:  */
/*                                                     +:+ +:+         +:+    */
/*   By: YOUR_LOGIN <YOUR_LOGIN@student.1337.ma>     +#+  +:+       +#+       */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by YOUR_LOGIN        #+#    #+#             */
/*   Updated: 2026/09/12 00:00:00 by YOUR_LOGIN       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	compute_priority(t_coder *coder)
{
	long	priority;

	if (coder->env->args.scheduler == FIFO)
		priority = get_time_ms();
	else
	{
		pthread_mutex_lock(&(coder->env->state_mutex));
		priority = coder->last_compile_start
			+ coder->env->args.time_to_burnout;
		pthread_mutex_unlock(&(coder->env->state_mutex));
	}
	return (priority);
}

static t_qnode	*find_node(t_queue *q, int coder_id)
{
	int	i;

	i = 0;
	while (i < q->size)
	{
		if (q->array[i].coder->id == coder_id)
			return (&q->array[i]);
		i++;
	}
	return (NULL);
}

static int	shares_dongle(t_coder *a, t_coder *b)
{
	if (a->left_dongle == b->left_dongle || a->left_dongle == b->right_dongle)
		return (1);
	if (a->right_dongle == b->left_dongle || a->right_dongle == b->right_dongle)
		return (1);
	return (0);
}

/*
** A coder may attempt to take its dongles as soon as it is the
** highest-priority (per scheduler policy) claimant of BOTH of its own
** dongles - not only when it is strictly first in the whole building.
** With a ring of coders, at most 2 neighbouring coders ever compete
** for the same dongle, so this lets non-conflicting coders elsewhere
** on the table proceed in parallel instead of queueing behind an
** unrelated request, while still granting each individual dongle in
** the exact order the chosen scheduler defines. With a single coder,
** left_dongle == right_dongle (only one physical dongle exists): two
** hands can never be filled from one dongle, so that coder must never
** "succeed" here - it is meant to burn out, exactly as the subject's
** single-coder reference case requires.
*/
int	is_our_turn(t_coder *coder)
{
	t_queue	*q;
	t_qnode	*us;
	int		i;

	q = coder->env->wait_queue;
	if (coder->left_dongle == coder->right_dongle)
		return (0);
	us = find_node(q, coder->id);
	if (!us)
		return (0);
	i = 0;
	while (i < q->size)
	{
		if (q->array[i].coder->id != coder->id
			&& shares_dongle(coder, q->array[i].coder)
			&& node_before(&q->array[i], us))
			return (0);
		i++;
	}
	return (1);
}

/*
** Combines priority computation, the arbitration check and the atomic
** take of both dongles into one call for the polling loop. Pushes the
** coder into the queue on the first call only.
*/
int	attempt_acquire(t_coder *coder, int *pushed)
{
	int	can_take;

	if (!(*pushed))
	{
		push_queue(coder->env->wait_queue, coder, compute_priority(coder));
		*pushed = 1;
	}
	can_take = 0;
	pthread_mutex_lock(&(coder->env->wait_queue->q_mutex));
	if (is_our_turn(coder))
		can_take = try_take_dongles(coder);
	pthread_mutex_unlock(&(coder->env->wait_queue->q_mutex));
	return (can_take);
}
