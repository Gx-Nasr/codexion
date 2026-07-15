/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:53:47 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/15 16:54:04 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int ac, char **av)
{
	t_data	data;
	t_sim	sim;

	if (ac != 9)
		return (printf("error\n"), 1);
	if (!data_init(av, &data))
		return (printf("error\n"), 1);
	if (!sim_init(&sim, &data))
		return (1);
	if (!creat_threads(sim.coders, data.number_of_coders))
		return (1);
	sim.start_time = get_time_ms();
	sim.start = 1;
	monitor(&sim);
	cleaner(&sim, 0, 0);
	return (0);
}
