/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:55:56 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/24 18:20:00 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	wait_to_take_dongle(t_dongle *dongle, t_coder *coder)
{
	struct timespec		t_wait;

	while (dongle->available_at > get_time_ms()
		|| dongle->queue[0].id != coder->id || dongle->is_taken)
	{
		if (dongle->queue[0].id != coder->id || dongle->is_taken)
			pthread_cond_wait(&dongle->d_cond, &dongle->d_mutex);
		if (end_checker(coder->sim))
		{
			pthread_mutex_unlock(&dongle->d_mutex);
			return (0);
		}
		if (dongle->available_at > get_time_ms())
		{
			cond_wait_time(&t_wait, dongle->available_at);
			pthread_cond_timedwait(&dongle->d_cond,
				&dongle->d_mutex, &t_wait);
		}
		if (end_checker(coder->sim))
		{
			pthread_mutex_unlock(&dongle->d_mutex);
			return (0);
		}
	}
	return (1);
}

int	take_dongel(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->d_mutex);
	if (!wait_to_take_dongle(dongle, coder))
		return (0);
	dongle->is_taken = 1;
	pop_req(dongle);
	pthread_mutex_unlock(&dongle->d_mutex);
	return (1);
}

void	put_dongles(t_dongle *l_dongle,
	t_dongle *r_dongle, int cooldown_t)
{
	pthread_mutex_lock(&l_dongle->d_mutex);
	l_dongle->is_taken = 0;
	l_dongle->available_at = get_time_ms() + cooldown_t;
	pthread_cond_broadcast(&l_dongle->d_cond);
	pthread_mutex_unlock(&l_dongle->d_mutex);
	pthread_mutex_lock(&r_dongle->d_mutex);
	r_dongle->is_taken = 0;
	r_dongle->available_at = get_time_ms() + cooldown_t;
	pthread_cond_broadcast(&r_dongle->d_cond);
	pthread_mutex_unlock(&r_dongle->d_mutex);
}

int	do_the_routine(t_coder *coder)
{
	int		cooldown_t;

	cooldown_t = coder->sim->data.dongle_cooldown;
	ft_compile(coder);
	if (!log_compiling(coder))
		return (0);
	if (!sleepr(coder->sim->data.time_to_compile, coder->sim))
		return (0);
	put_dongles(coder->left_dongle,
		coder->right_dongle, cooldown_t);
	if (!print_log("is debugging", coder, 0))
		return (0);
	if (!sleepr(coder->sim->data.time_to_debug, coder->sim))
		return (0);
	if (!print_log("is refactoring", coder, 0))
		return (0);
	if (!sleepr(coder->sim->data.time_to_refactor, coder->sim))
		return (0);
	return (1);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	ft_start(coder->sim);
	pthread_mutex_lock(&coder->c_mutex);
	coder->last_compile_t = get_time_ms();
	pthread_mutex_unlock(&coder->c_mutex);
	if (coder->id % 2)
		usleep(100);
	while (!end_checker(coder->sim))
	{
		request_dongels(coder, coder->left_dongle, coder->right_dongle);
		if (!take_dongel(coder, coder->left_dongle)
			|| !take_dongel(coder, coder->right_dongle))
			break ;
		if (!do_the_routine(coder))
			break ;
		if (coder->compile_count >= coder->sim->data.n_of_compiles_req)
			break ;
	}
	return (NULL);
}
