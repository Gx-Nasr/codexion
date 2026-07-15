/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 06:46:59 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/15 16:51:25 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	finish(t_sim *sim)
{
	pthread_mutex_lock(&sim->s_mutex);
	sim->is_finished = 1;
	pthread_mutex_unlock(&sim->s_mutex);
}

int	monitor_checker(t_coder *coder)
{
	long long	burnout_t;

	burnout_t = coder->sim->data.time_to_burnout;
	pthread_mutex_lock(&coder->c_mutex);
	if (get_time_ms() > coder->last_compile_t + burnout_t)
	{
		finish(coder->sim);
		printf("%lld %d burned out\n", get_time_ms()
			- coder->sim->start_time, coder->id);
		pthread_mutex_unlock(&coder->c_mutex);
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

	pthread_mutex_lock(&sim->s_mutex);
	while (!sim->is_finished)
	{
		pthread_mutex_unlock(&sim->s_mutex);
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
		pthread_mutex_lock(&sim->s_mutex);
	}
	pthread_mutex_unlock(&sim->s_mutex);
}
