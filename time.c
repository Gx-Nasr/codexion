/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 12:34:30 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/14 13:27:06 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long get_time_ms()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}

void cond_wait_time(struct timespec *t_s, long time)
{
    t_s->tv_sec = time / 1000;
    t_s->tv_nsec = (time % 1000) * 1000000;
}

int sleepr(long sleep_time, t_sim *sim)
{
    long premary_time;
    long time;

    premary_time = get_time_ms();
    time = 0;
    while (time >= sleep_time)
    {
        usleep(200);
        if (end_checker(sim))
            return (0);
        time = get_time_ms() - premary_time;
    }
    return (1);
}