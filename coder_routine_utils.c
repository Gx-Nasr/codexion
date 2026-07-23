/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 13:59:11 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/23 13:24:34 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	print_log(char *str, t_coder *coder, int is_monitor)
{
	long long	print_time;
	int			id;

	pthread_mutex_lock(&coder->sim->print_mutex);
	pthread_mutex_lock(&coder->sim->s_mutex);
	print_time = get_time_ms() - coder->sim->start_time;
	pthread_mutex_unlock(&coder->sim->s_mutex);
	id = coder->id;
	if (!is_monitor)
	{
		if (end_checker(coder->sim))
		{
			pthread_mutex_unlock(&coder->sim->print_mutex);
			return (0);
		}	
	}
	printf("%lld %d %s\n", print_time, id, str);
	pthread_mutex_unlock(&coder->sim->print_mutex);
	return (1);
}

void	ft_start(t_sim *sim)
{
	while (1)
	{
		pthread_mutex_lock(&sim->s_mutex);
		if (sim->start)
		{
			pthread_mutex_unlock(&sim->s_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->s_mutex);
		usleep(50);
	}
}

void	ft_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->c_mutex);
	coder->compile_count += 1;
	coder->last_compile_t = get_time_ms();
	pthread_mutex_unlock(&coder->c_mutex);
}

int	log_compiling(t_coder *coder)
{
	long long	print_time;
	int			id;

	pthread_mutex_lock(&coder->sim->print_mutex);
	pthread_mutex_lock(&coder->sim->s_mutex);
	print_time = get_time_ms() - coder->sim->start_time;
	pthread_mutex_unlock(&coder->sim->s_mutex);
	id = coder->id;
	if (end_checker(coder->sim))
	{
		pthread_mutex_unlock(&coder->sim->print_mutex);
		return (0);
	}
	printf("%lld %d has taken a dongle\n", print_time, id);
	printf("%lld %d has taken a dongle\n", print_time, id);
	printf("%lld %d is compiling\n", print_time, id);
	pthread_mutex_unlock(&coder->sim->print_mutex);
	return (1);
}

void	request_dongels(t_coder *coder, t_dongle *l_dongle, t_dongle *r_dongle)
{
	t_request			request;
	int					burnout_time;

	pthread_mutex_lock(&l_dongle->d_mutex);
	pthread_mutex_lock(&coder->c_mutex);
	request.id = coder->id;
	burnout_time = coder->sim->data.time_to_burnout;
	request.preorety = coder->last_compile_t + burnout_time - get_time_ms();
	pthread_mutex_unlock(&coder->c_mutex);
	push_req(&request, l_dongle, coder->sim->data.is_edf);
	pthread_mutex_unlock(&l_dongle->d_mutex);
	pthread_mutex_lock(&r_dongle->d_mutex);
	pthread_mutex_lock(&coder->c_mutex);
	request.id = coder->id;
	burnout_time = coder->sim->data.time_to_burnout;
	request.preorety = coder->last_compile_t + burnout_time - get_time_ms();
	pthread_mutex_unlock(&coder->c_mutex);
	push_req(&request, r_dongle, coder->sim->data.is_edf);
	pthread_mutex_unlock(&r_dongle->d_mutex);
}
