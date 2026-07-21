/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 06:46:59 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/21 08:51:19 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	finish(t_sim *sim)
{
	int	i;

	i = 0;
	pthread_mutex_lock(&sim->s_mutex);
	sim->is_finished = 1;
	pthread_mutex_unlock(&sim->s_mutex);
	while (i < sim->data.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].d_mutex);
		pthread_cond_broadcast(&sim->dongles[i].d_cond);
		pthread_mutex_unlock(&sim->dongles[i].d_mutex);
		++i;
	}
}

int	monitor_checker(t_coder *coder)
{
	long long	burnout_t;

	burnout_t = coder->sim->data.time_to_burnout;
	pthread_mutex_lock(&coder->c_mutex);
	if (get_time_ms() > coder->last_compile_t + burnout_t)
	{
		pthread_mutex_unlock(&coder->c_mutex);
		finish(coder->sim);
		print_log("burned out", coder, 1);
		return (-1);
	}
	if (coder->compile_count < coder->sim->data.n_of_compiles_req)
	{
		pthread_mutex_unlock(&coder->c_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->c_mutex);
	return (0);
}

void	monitor(t_sim *sim)
{
	int	i;
	int	res;
	int	the_end;

	while (1)
	{
		i = 0;
		the_end = 1;
		while (i < sim->data.number_of_coders)
		{
			res = monitor_checker(&sim->coders[i++]);
			if (res == -1)
				return ;
			else if (res)
				the_end = 0;
		}
		if (the_end)
			finish(sim);
		usleep(1000);
		if (end_checker(sim))
			return ;
	}
}
