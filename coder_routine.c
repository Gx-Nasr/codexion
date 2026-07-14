/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:55:56 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/14 06:51:40 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int end_checker(t_sim *sim)
{
    int is_finished;

    pthread_mutex_lock(sim->is_finished);
    is_finished = sim->is_finished;
    pthread_mutex_unlock(sim->is_finished);

    return is_finished;
}

int take_dongel(t_coder *coder, t_dongle *dongle)
{
    t_request request;
    int burnout_time;

    burnout_time = coder->sim->data.time_to_burnout;
    request.id = coder->id;
    request.preorety =  coder->last_compile_t + burnout_time - get_time_ms();
    pthread_mutex_lock(&dongle->d_mutex);
    push_req(&request, coder->left_dongle, coder->sim->data.is_edf);
    while (dongle->available_at > get_time_ms() || dongle->queue[0].id != coder->id || dongle->is_taken)
    {
        if (dongle->queue[0].id != coder->id || dongle->is_taken)
            pthread_cond_wait(&dongle->d_cond, &dongle->d_mutex);
        if (dongle->available_at > get_time_ms())
        {
            struct timespec t_wait;
            cond_wait_time(&t_wait, dongle->available_at);
            pthread_cond_timedwait(&dongle->d_cond, &dongle->d_mutex, &t_wait);
        }
        if (end_checker(coder->sim))
            return (0);
    }
    dongle->is_taken = 1;
    pop_req(dongle);
    pthread_mutex_unlock(&dongle->d_mutex);
    return(1);
}

void *coder_routine(void *arg)
{
    t_coder *coder;

    coder = (t_coder *)arg;
    while (!end_checker(coder->sim))
    {
        
    }
    
    return (NULL);
}

// reaquest dongle:
// katpushi fl heap 

// check_sim = fucntion read a variabl li kayn f sim li smyto is finished return; 
// idan kan while 3la sim wach salat ola ba9i



// while (check_sim)
// {
//     /* code */
// }
