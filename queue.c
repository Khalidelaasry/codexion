/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                         :+:      :+:    :+:     */
/*                                                     +:+ +:+         +:+    */
/*   By: khelaasr <khelaasr@student.1337.ma>         +#+  +:+       +#+      */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by khelaasr #+#    #+#                     */
/*   Updated: 2026/09/12 00:00:00 by khelaasr ###   ########.fr              */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_queue(t_env *env)
{
	int	cap;

	cap = env->args.num_coders;
	env->wait_queue = malloc(sizeof(t_queue));
	if (!env->wait_queue)
		return (1);
	env->wait_queue->capacity = cap;
	env->wait_queue->size = 0;
	env->wait_queue->seq_counter = 0;
	env->wait_queue->array = malloc(sizeof(t_qnode) * cap);
	if (!env->wait_queue->array)
		return (1);
	if (pthread_mutex_init(&(env->wait_queue->q_mutex), NULL) != 0)
		return (1);
	return (0);
}

void	push_queue(t_queue *q, t_coder *coder, long priority)
{
	pthread_mutex_lock(&(q->q_mutex));
	if (q->size == q->capacity)
	{
		pthread_mutex_unlock(&(q->q_mutex));
		return ;
	}
	q->array[q->size].coder = coder;
	q->array[q->size].priority = priority;
	q->array[q->size].seq = q->seq_counter++;
	q->size++;
	heapify_up(q, q->size - 1);
	pthread_mutex_unlock(&(q->q_mutex));
}
