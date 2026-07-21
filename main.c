/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:53:47 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/21 11:17:59 by nel-adao         ###   ########.fr       */
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
		return (printf("Usage: ./codexion <number_of_coders> <time_to_burnout> "
				"<time_to_compile> <time_to_debug> <time_to_refactor> "
				"<number_of_compiles> <dongle_cooldown> <fifo|edf>\n"), 1);
	if (!data_init(av, &data))
		return (printf("Error: Invalid input values.\n"), 1);
	if (!sim_init(&sim, &data))
		return (printf("Error: Simulation initialization failed"
				" (memory or mutex error).\n"), 1);
	if (!creat_threads(sim.coders, data.number_of_coders))
		return (printf("Error: pthread_create() failed.\n"), 1);
	run_threads(&sim);
	monitor(&sim);
	while (i < data.number_of_coders)
		pthread_join(sim.coders[i++].thread, NULL);
	cleaner(&sim, i - 1);
	return (0);
}
