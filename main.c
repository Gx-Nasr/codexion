/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:53:47 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/21 08:55:50 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	run_threads(t_sim *sim)
{
	pthread_mutex_lock(&sim->s_mutex);
	sim->start_time = get_time_ms();
	sim->start = 1;
	pthread_mutex_unlock(&sim->s_mutex);
}

int	main(int ac, char **av)
{
	t_data	data;
	t_sim	sim;
	int		i;

	i = 0;
	if (ac != 9)
		return (printf("error\n"), 1);
	if (!data_init(av, &data))
		return (printf("error\n"), 1);
	if (!sim_init(&sim, &data))
		return (1);
	if (!creat_threads(sim.coders, data.number_of_coders))
		return (1);
	run_threads(&sim);
	monitor(&sim);
	while (i < data.number_of_coders)
		pthread_join(sim.coders[i++].thread, NULL);
	cleaner(&sim, i - 1);
	return (0);
}
