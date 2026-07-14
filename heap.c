/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 09:44:10 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/13 15:35:26 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long get_time_ms()
{
    
}

void sort_edf(t_dongle *dongle)
{
    long pre0;
    long pre1;
    t_request tmp;

    pre0 = dongle->queue[0].preorety;
    pre1 = dongle->queue[1].preorety;
    tmp = dongle->queue[0];
        
    if (dongle->size_queue == 2)
    {
        if (pre0 > pre1)
            dongle->queue[0] = dongle->queue[1];
            dongle->queue[1] = tmp;
    }
}