/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:55:56 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/13 16:18:49 by nel-adao         ###   ########.fr       */
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
void push_req(t_request *request, t_dongle *dongle, int is_edf)
{
    if (dongle->size_queue == 0)
        dongle->queue[0] = *request;
    else if(dongle->size_queue == 1)
        dongle->queue[1] = *request;

    dongle->size_queue += 1;
    if (is_edf)
        sort_edf(dongle);
}



int request_dongel(t_coder *coder, t_dongle *dongle)
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
        
    }
    
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
