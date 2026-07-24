/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 09:44:10 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/24 18:00:11 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	pop_req(t_dongle *dongle)
{
	t_request	tmp;

	dongle->size_queue -= 1;
	tmp = dongle->queue[0];
	dongle->queue[0] = dongle->queue[1];
	dongle->queue[1] = tmp;
}

void	push_req(t_request *request, t_dongle *dongle, int is_edf)
{
	if (dongle->size_queue == 0)
		dongle->queue[0] = *request;
	else if (dongle->size_queue == 1)
		dongle->queue[1] = *request;
	dongle->size_queue += 1;
	if (is_edf)
		sort_edf(dongle);
}

void	sort_edf(t_dongle *dongle)
{
	long long	pre0;
	long long	pre1;
	t_request	tmp;

	pre0 = dongle->queue[0].preorety;
	pre1 = dongle->queue[1].preorety;
	tmp = dongle->queue[0];
	if (dongle->size_queue == 2)
	{
		if (pre0 > pre1)
		{
			dongle->queue[0] = dongle->queue[1];
			dongle->queue[1] = tmp;
		}
	}
}
