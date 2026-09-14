/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                          :+:      :+:    :+:     */
/*                                                     +:+ +:+         +:+    */
/*   By: khelaasr <khelaasr@student.1337.ma>         +#+  +:+       +#+      */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by khelaasr #+#    #+#                     */
/*   Updated: 2026/09/12 00:00:00 by khelaasr ###   ########.fr              */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	swap_nodes(t_qnode *a, t_qnode *b)
{
	t_qnode	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

/*
** Strict ordering used by the heap: lower priority wins; on an exact
** priority tie (two coders requesting at the same millisecond, or two
** identical EDF deadlines) the lower sequence number - whoever asked
** first - wins. This guarantees a fully deterministic scheduling
** order, as required by the subject for the EDF tie-breaker.
*/
int	node_before(t_qnode *a, t_qnode *b)
{
	if (a->priority != b->priority)
		return (a->priority < b->priority);
	return (a->seq < b->seq);
}

void	heapify_up(t_queue *q, int index)
{
	int	parent;

	if (index > 0 && index < q->size)
	{
		parent = (index - 1) / 2;
		if (node_before(&q->array[index], &q->array[parent]))
		{
			swap_nodes(&q->array[index], &q->array[parent]);
			heapify_up(q, parent);
		}
	}
}

void	heapify_down(t_queue *q, int index)
{
	int	left;
	int	right;
	int	smallest;

	left = 2 * index + 1;
	right = 2 * index + 2;
	smallest = index;
	if (left < q->size && node_before(&q->array[left], &q->array[smallest]))
		smallest = left;
	if (right < q->size && node_before(&q->array[right], &q->array[smallest]))
		smallest = right;
	if (smallest != index)
	{
		swap_nodes(&q->array[index], &q->array[smallest]);
		heapify_down(q, smallest);
	}
}

/*
** Removes the node belonging to coder_id from anywhere in the heap
** (not just the root), assuming the caller already holds q_mutex.
** This is what lets a coder that is not at the strict front of the
** whole queue still be serviced as soon as it is the highest-priority
** claimant of BOTH of its own dongles (see is_our_turn() in
** coder_utils.c), instead of forcing every coder in the building to
** wait behind one unrelated, currently-blocked request.
*/
void	remove_node(t_queue *q, int coder_id)
{
	int	i;
	int	parent;

	i = 0;
	while (i < q->size && q->array[i].coder->id != coder_id)
		i++;
	if (i >= q->size)
		return ;
	q->size--;
	q->array[i] = q->array[q->size];
	parent = (i - 1) / 2;
	if (i > 0 && node_before(&q->array[i], &q->array[parent]))
		heapify_up(q, i);
	else
		heapify_down(q, i);
}
