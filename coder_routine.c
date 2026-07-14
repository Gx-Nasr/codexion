/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:55:56 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/14 14:09:26 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int ft_compile(t_coder *coder)
{
    pthread_mutex_lock(&coder->c_mutex);
    coder->compile_count += 1;
    coder->last_compile_t = get_time_ms();
    if (coder->compile_count == coder->sim->data.n_of_compiles_req)
    {
        pthread_mutex_unlock(&coder->c_mutex);
        return (1);
    }
    pthread_mutex_unlock(&coder->c_mutex);
    return (0);
}

int end_checker(t_sim *sim)
{
    int is_finished;

    pthread_mutex_lock(sim->is_finished);
    is_finished = sim->is_finished;
    pthread_mutex_unlock(sim->is_finished);

    return is_finished;
}

int take_dongel(t_coder *coder, t_dongle *dongle, int is_left)
{
    t_request request;
    int burnout_time;

    burnout_time = coder->sim->data.time_to_burnout;
    request.id = coder->id;
    request.preorety =  coder->last_compile_t + burnout_time - get_time_ms();
    pthread_mutex_lock(&dongle->d_mutex);
    push_req(&request, dongle, coder->sim->data.is_edf);
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
    printf("%ld %d has taken a dongle", get_time_ms() - coder->sim->start_time, coder->id);
    pthread_mutex_unlock(&dongle->d_mutex);
    return(1);
}

void put_dongles(t_dongle *l_dongle, t_dongle *r_dongle, int cooldown_t)
{
    pthread_mutex_lock(&l_dongle->d_mutex);
    l_dongle->is_taken = 0;
    l_dongle->available_at = get_time_ms() + cooldown_t;
    pthread_cond_broadcast(&l_dongle->d_cond);
    pthread_mutex_unlock(&l_dongle->d_mutex);

    pthread_mutex_lock(&r_dongle->d_mutex);
    r_dongle->is_taken = 0;
    l_dongle->available_at = get_time_ms() + cooldown_t;
    pthread_cond_broadcast(&r_dongle->d_cond);
    pthread_mutex_unlock(&r_dongle->d_mutex);
}

void *coder_routine(void *arg)
{
    t_coder *coder;
    int cooldown_t;

    coder = (t_coder *)arg;
    cooldown_t = coder->sim->data.dongle_cooldown;
    while (!end_checker(coder->sim))
    {
        if (!take_dongel(coder, coder->left_dongle, 1) || !take_dongel(coder, coder->right_dongle, 0))
            return ;
        printf("%ld %d is compiling", get_time_ms() - coder->sim->start_time, coder->id);
        if (!sleepr(coder->sim->data.time_to_compile, coder->sim))
            return;
        if (ft_compile(coder))
            return ;
        put_dongles(coder->left_dongle, coder->right_dongle, cooldown_t);
        printf("%ld %d is debugging", get_time_ms() - coder->sim->start_time, coder->id);
        if (!sleepr(coder->sim->data.time_to_debug, coder->sim))
            return ;
        printf("%ld %d is refactoring", get_time_ms() - coder->sim->start_time, coder->id);
        if (!sleepr(coder->sim->data.time_to_refactor, coder->sim))
            return ;
    }

    return ;
}
