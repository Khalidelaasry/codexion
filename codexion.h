/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khelaasr <khelaasr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 18:33:32 by khelaasr          #+#    #+#             */
/*   Updated: 2026/09/14 18:34:12 by khelaasr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# define _DEFAULT_SOURCE
# define _POSIX_C_SOURCE 200809L

# include <pthread.h>
# include <time.h>
# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_args
{
	int				num_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				num_compiles_required;
	long			dongle_cooldown;
	t_scheduler		scheduler;
}	t_args;

typedef struct s_env	t_env;
typedef struct s_coder	t_coder;

typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	mutex;
	long			last_used_time;
	int				is_available;
}	t_dongle;

typedef struct s_qnode
{
	t_coder			*coder;
	long			priority;
	long			seq;
}	t_qnode;

typedef struct s_queue
{
	t_qnode			*array;
	int				size;
	int				capacity;
	long			seq_counter;
	pthread_mutex_t	q_mutex;
}	t_queue;

struct s_coder
{
	int				id;
	pthread_t		thread_id;
	long			last_compile_start;
	int				compiles_count;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	t_env			*env;
};

struct s_env
{
	t_args			args;
	t_coder			*coders;
	t_dongle		*dongles;
	t_queue			*wait_queue;
	pthread_mutex_t	write_mutex;
	pthread_mutex_t	state_mutex;
	long			start_time;
	int				simulation_stop;
};

long		get_time_ms(void);
void		ft_usleep(long time_to_wait_ms, t_env *env);
void		ft_log(t_env *env, int id, const char *state);
int			init_env(t_env *env, int argc, char **argv);
int			parse_args(t_args *args, int argc, char **argv);
int			init_queue(t_env *env);
void		push_queue(t_queue *q, t_coder *coder, long priority);
void		swap_nodes(t_qnode *a, t_qnode *b);
int			node_before(t_qnode *a, t_qnode *b);
void		heapify_up(t_queue *q, int index);
void		heapify_down(t_queue *q, int index);
void		remove_node(t_queue *q, int coder_id);
void		*coder_routine(void *arg);
int			is_our_turn(t_coder *coder);
int			attempt_acquire(t_coder *coder, int *pushed);
int			try_take_dongles(t_coder *coder);
void		*monitor_routine(void *arg);
void		free_all(t_env *env);

#endif
