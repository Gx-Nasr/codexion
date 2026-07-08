/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 11:20:39 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/08 09:36:45 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int coders_and_dongles_init(t_sim *sim, t_data *data)
{
    int i;
    t_coder *coders = malloc(data->number_of_coders * sizeof(t_coder));
    if (!coders)
        return (0);
    
    i = 0;
    while (i < data->number_of_coders)
    {
        coders[i].sim = sim;
        coders[i].id = i;
        coders[i].compile_count = 0;
        ++i;
    }
    sim->coders = coders;
    return (1);
}

int dongles_init(t_sim *sim, t_data *data)
{
    int i;
    t_dongle *dongles = malloc(data->number_of_coders * sizeof(t_dongle));
    if (!dongles)
        return (0);

    i = 0;
    while (i < data->number_of_coders)
    {
        dongles[i].is_taken = 0;
        dongles[i].size_queue = 0;
        ++i;
    }
    sim->dongles = dongles;
    return (1);
}

int sim_init(t_sim *sim, t_data *data)
{
    sim->data = *data;
    if (!dongles_init(sim, data))
        return (0);
    if (!coders_init(sim, data))
        return (free(sim->dongles), 0);
    sim->is_finished = 0;
}