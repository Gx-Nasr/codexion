/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:53:47 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/15 14:15:33 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


int per(char *s)
{
    printf("%s", s);
    return (1);
}

int main(int ac, char **av)
{
    t_data data;
    t_sim sim;

    if (ac != 9)
        return per("error");
    if (!data_init(av, &data))
        return per("error");
    if (!sim_init(&sim, &data))
        return (1);
    if (!creat_threads(sim.coders, data.number_of_coders))
        return (1);
    sim.start_time = get_time_ms();
    sim.start = 1;
    int i = 0;
    while (i < sim.data.number_of_coders)
    {
        pthread_join(sim.coders[i].thread, NULL);
        ++i;
    }
    cleaner(&sim, 0, 0);
    return 0;
}
