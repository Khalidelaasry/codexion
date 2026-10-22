/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khelaasr <khelaasr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 12:00:00 by khelaasr          #+#    #+#             */
/*   Updated: 2026/10/22 12:00:00 by khelaasr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_number(const char *text, long limit, long *result)
{
	long	number;
	long	maximum_prefix;
	int		digit;
	size_t	i;

	if (text[0] == '\0')
		return (0);
	number = 0;
	i = 0;
	while (text[i] != '\0')
	{
		if (text[i] < '0' || text[i] > '9')
			return (0);
		digit = text[i] - '0';
		maximum_prefix = (limit - digit) / 10;
		if (number > maximum_prefix)
			return (0);
		number = number * 10 + digit;
		i++;
	}
	result[0] = number;
	return (1);
}

static void	store_numbers(t_args *args, long *values)
{
	args->num_coders = (int)values[0];
	args->time_to_burnout = values[1];
	args->time_to_compile = values[2];
	args->time_to_debug = values[3];
	args->time_to_refactor = values[4];
	args->num_compilations = (int)values[5];
	args->dongle_cooldown = values[6];
}

static int	report_invalid_number(int position, const char *text)
{
	fprintf(stderr, "Error: invalid numeric argument %d: '%s'\n",
		position, text);
	return (0);
}

static int	parse_numbers(char **argv, t_args *args)
{
	long	values[7];
	long	limit;
	int		i;

	i = 0;
	while (i < 7)
	{
		limit = LONG_MAX;
		if (i == 0 || i == 5)
			limit = INT_MAX;
		if (parse_number(argv[i + 1], limit, &values[i]) == 0)
			return (report_invalid_number(i + 1, argv[i + 1]));
		if (i == 0 && values[i] == 0)
			return (report_invalid_number(i + 1, argv[i + 1]));
		i++;
	}
	store_numbers(args, values);
	return (1);
}

int	parse_args(int argc, char **argv, t_args *args)
{
	if (argc != 9)
	{
		fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n");
		return (0);
	}
	if (parse_numbers(argv, args) == 0)
		return (0);
	if (strcmp(argv[8], "fifo") == 0)
		args->scheduler = FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		args->scheduler = EDF;
	else
	{
		fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'\n");
		return (0);
	}
	return (1);
}
