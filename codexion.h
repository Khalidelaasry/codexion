/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khelaasr <khelaasr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 12:00:00 by khelaasr          #+#    #+#             */
/*   Updated: 2026/10/22 12:00:00 by khelaasr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# define _DEFAULT_SOURCE
# define _POSIX_C_SOURCE 200809L

# include <pthread.h>
# include <stdio.h>
# include <sys/time.h>
# include <time.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <limits.h>

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
	int				num_compilations;
	long			dongle_cooldown;
	t_scheduler		scheduler;
}	t_args;

typedef struct s_env		t_env;
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

int						parse_args(int argc, char **argv, t_args *args);

#endif
