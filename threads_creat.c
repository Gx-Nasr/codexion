/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads_creat.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/11 16:31:29 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/16 15:30:57 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleaner(t_sim *sim, int i)
{
	while (i >= 0)
	{
		pthread_mutex_destroy(&sim->coders[i].c_mutex);
		pthread_mutex_destroy(&sim->dongles[i].d_mutex);
		pthread_cond_destroy(&sim->dongles[i].d_cond);
		--i;
	}
	free(sim->coders);
	free(sim->dongles);
	pthread_mutex_destroy(&sim->s_mutex);
}

int	creat_threads(t_coder *coders, int n_coders)
{
	int	i;
	int	j;

	i = 0;
	while (i < n_coders)
	{
		if (pthread_create(&coders[i].thread, NULL,
				coder_routine, &coders[i]) != 0)
		{
			pthread_mutex_lock(&coders[i].sim->s_mutex);
			coders[i].sim->is_finished = 1;
			pthread_mutex_unlock(&coders[i].sim->s_mutex);
			j = i;
			while (j--)
				pthread_join(coders[j].thread, NULL);
			cleaner(coders[0].sim, i - 1);
			return (0);
		}
		++i;
	}
	return (1);
}
