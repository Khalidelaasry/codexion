/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                              :+:      :+:    :+:  */
/*                                                     +:+ +:+         +:+    */
/*   By: YOUR_LOGIN <YOUR_LOGIN@student.1337.ma>     +#+  +:+       +#+       */
/*                                                 +#+#+#+#+#+   +#+          */
/*   Created: 2026/09/12 00:00:00 by YOUR_LOGIN        #+#    #+#             */
/*   Updated: 2026/09/12 00:00:00 by YOUR_LOGIN       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Converts a string to a long integer, strictly. Rejects negative
** numbers, non-digit characters and empty strings by returning -1
** (every argument of this project must be a non-negative integer).
*/
static long	ft_atol(const char *str)
{
	long	res;
	int		i;
	int		digits;

	res = 0;
	i = 0;
	digits = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-')
		return (-1);
	if (str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = (res * 10) + (str[i] - '0');
		i++;
		digits++;
	}
	if (str[i] != '\0' || digits == 0)
		return (-1);
	return (res);
}

static int	check_ranges(t_args *args)
{
	if (args->num_coders <= 0 || args->time_to_burnout < 0
		|| args->time_to_compile < 0 || args->time_to_debug < 0
		|| args->time_to_refactor < 0 || args->num_compiles_required < 0
		|| args->dongle_cooldown < 0)
	{
		printf("Error: Arguments must be non-negative integers\n");
		return (1);
	}
	return (0);
}

/*
** Parses the command line arguments and fills the t_args structure.
** Returns 0 on success, 1 on error.
*/
int	parse_args(t_args *args, int argc, char **argv)
{
	if (argc != 9)
	{
		printf("Error: Invalid number of arguments\n");
		return (1);
	}
	args->num_coders = (int)ft_atol(argv[1]);
	args->time_to_burnout = ft_atol(argv[2]);
	args->time_to_compile = ft_atol(argv[3]);
	args->time_to_debug = ft_atol(argv[4]);
	args->time_to_refactor = ft_atol(argv[5]);
	args->num_compiles_required = (int)ft_atol(argv[6]);
	args->dongle_cooldown = ft_atol(argv[7]);
	if (check_ranges(args) != 0)
		return (1);
	if (strcmp(argv[8], "fifo") == 0)
		args->scheduler = FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		args->scheduler = EDF;
	else
	{
		printf("Error: Scheduler must be exactly 'fifo' or 'edf'\n");
		return (1);
	}
	return (0);
}
