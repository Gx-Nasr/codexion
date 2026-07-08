/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:53:47 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/06 10:56:31 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


int print_error(char *s)
{
    printf("%s", s);
    return (1);
}

int main(int ac, char **av)
{
    t_data data;
    t_sim sim;

    if (ac != 9)
        return print_error("error");

    if (!data_init(av, &data))
        return print_error("error");

    return 0;
}
