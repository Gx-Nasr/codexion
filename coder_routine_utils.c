/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 13:59:11 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/16 14:03:04 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_log(char *str, t_coder *coder)
{
	printf("%lld %d %s\n", get_time_ms()
		- coder->sim->start_time, coder->id + 1, str);
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
