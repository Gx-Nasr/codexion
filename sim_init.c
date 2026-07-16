/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 11:20:39 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/16 13:55:15 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	coder_helper(t_coder *coder, t_sim *sim, int id)
{
	coder->sim = sim;
	coder->id = id;
	coder->compile_count = 0;
	coder->last_compile_t = get_time_ms();
}

int	coders_init(t_sim *sim, t_data *data)
{
	int		i;
	t_coder	*coders;

	coders = malloc(data->number_of_coders * sizeof(t_coder));
	if (!coders)
		return (0);
	i = 0;
	while (i < data->number_of_coders)
	{
		coder_helper(&coders[i], sim, i);
		coders[i].left_dongle = &sim->dongles[i];
		coders[i].right_dongle = &sim->dongles[(i + 1)
			% data->number_of_coders];
		if (pthread_mutex_init(&coders[i].c_mutex, NULL) != 0)
			return (free(coders), 0);
		if (i % 2)
		{
			coders[i].right_dongle = &sim->dongles[i];
			coders[i].left_dongle = &sim->dongles[(i + 1)
				% data->number_of_coders];
		}
		++i;
	}
	sim->coders = coders;
	return (1);
}

int	dongles_init(t_sim *sim, t_data *data)
{
	int			i;
	t_dongle	*dongles;

	dongles = malloc(data->number_of_coders * sizeof(t_dongle));
	if (!dongles)
		return (0);
	i = 0;
	while (i < data->number_of_coders)
	{
		dongles[i].is_taken = 0;
		dongles[i].size_queue = 0;
		dongles[i].available_at = 0;
		if (pthread_mutex_init(&dongles[i].d_mutex, NULL) != 0)
			return (free(dongles), 0);
		if (pthread_cond_init(&dongles[i].d_cond, NULL) != 0)
			return (free(dongles), 0);
		++i;
	}
	sim->dongles = dongles;
	return (1);
}

int	sim_init(t_sim *sim, t_data *data)
{
	sim->data = *data;
	if (!dongles_init(sim, data))
		return (0);
	if (!coders_init(sim, data))
		return (free(sim->dongles), 0);
	if (pthread_mutex_init(&sim->s_mutex, NULL) != 0)
		return (free(sim->dongles), free(sim->coders), 0);
	sim->is_finished = 0;
	sim->start = 0;
	return (1);
}
