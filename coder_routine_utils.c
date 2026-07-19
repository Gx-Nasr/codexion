/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 13:59:11 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/19 10:45:06 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	print_log(char *str, t_coder *coder, int is_monitor)
{
	long long	print_time;
	int			id;

	pthread_mutex_lock(&coder->sim->print_mutex);
	print_time = get_time_ms() - coder->sim->start_time;
	id = coder->id + 1;
	if (!is_monitor)
	{
		if (end_checker(coder->sim))
		{
			pthread_mutex_unlock(&coder->sim->print_mutex);
			return 0;
		}	
	}
	printf("%lld %d %s\n", print_time, id, str);
	pthread_mutex_unlock(&coder->sim->print_mutex);
	return (1);
}

void	ft_start(t_sim *sim)
{
	while (!sim->start)
		usleep(200);
}

int	end_checker(t_sim *sim)
{
	int	is_finished;

	pthread_mutex_lock(&sim->s_mutex);
	is_finished = sim->is_finished;
	pthread_mutex_unlock(&sim->s_mutex);
	return (is_finished);
}

void	ft_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->c_mutex);
	coder->compile_count += 1;
	coder->last_compile_t = get_time_ms();
	pthread_mutex_unlock(&coder->c_mutex);
}
