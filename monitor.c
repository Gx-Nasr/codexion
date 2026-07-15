/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 06:46:59 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/15 15:42:06 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "codexion.h"

// int monitor(t_sim *sim)
// {
//     int i;
//     int the_end;

//     pthread_mutex_lock(&sim->s_mutex);
//     while (!sim->is_finished)
//     {
//         pthread_mutex_unlock(&sim->s_mutex);
//         i = 0;
//         the_end = 1;
//         while (i < sim->data.number_of_coders)
//         {
//             pthread_mutex_lock(&sim->coders[i]);
//             if (sim->coders[i].compile_count < sim->data.n_of_compiles_req)
//                 the_end = 0;
//             pthread_mutex_unlock(&sim->coders[i++]);
//         }
//         if (the_end)
//         {
//             pthread_mutex_lock(&sim->s_mutex);
//             sim->is_finished = 1;
//             pthread_mutex_unlock(&sim->s_mutex);
//         }
//         pthread_mutex_lock(&sim->s_mutex);
//     }
//     pthread_mutex_unlock(&sim->s_mutex);
// }
