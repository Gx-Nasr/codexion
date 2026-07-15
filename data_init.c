/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data_init.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:53:51 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/15 16:30:22 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	ft_is_edf(char *s)
{
	int		i;
	char	*fifo;
	char	*edf;

	i = 0;
	fifo = "fifo";
	edf = "edf";
	while (s[i])
	{
		if (s[i] != fifo[i])
			break ;
		++i;
	}
	if (!s[i] && !fifo[i])
		return (1);
	i = 0;
	while (s[i])
	{
		if (s[i] != edf[i])
			break ;
		++i;
	}
	if (!s[i] && !edf[i])
		return (0);
	return (-1);
}

static int	ft_atoi(char *s)
{
	long long	number;
	int			i;

	number = 0;
	i = 0;
	if (s[i] == '+')
		++i;
	while (s[i])
	{
		if (!(s[i] - '0' >= 0 && s[i] - '0' <= 9))
			return (-1);
		number = number * 10 + s[i] - '0';
		if (number > 2147483647)
			return (-1);
		++i;
	}
	return (number);
}

static int	data_checker(t_data *data)
{
	if (data->time_to_burnout < 0 || data->time_to_compile < 0)
		return (0);
	if (data->time_to_debug < 0 || data->time_to_refactor < 0)
		return (0);
	if (data->number_of_coders <= 0 || data->n_of_compiles_req <= 0)
		return (0);
	if (data->is_edf < 0 || data->dongle_cooldown < 0)
		return (0);
	return (1);
}

int	data_init(char **av, t_data *data)
{
	data->number_of_coders = ft_atoi(av[1]);
	data->time_to_burnout = ft_atoi(av[2]);
	data->time_to_compile = ft_atoi(av[3]);
	data->time_to_debug = ft_atoi(av[4]);
	data->time_to_refactor = ft_atoi(av[5]);
	data->n_of_compiles_req = ft_atoi(av[6]);
	data->dongle_cooldown = ft_atoi(av[7]);
	data->is_edf = ft_is_edf(av[8]);
	if (!data_checker(data))
		return (0);
	return (1);
}
