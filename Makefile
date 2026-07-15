# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/15 15:35:40 by nel-adao          #+#    #+#              #
#    Updated: 2026/07/15 15:39:32 by nel-adao         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion

CFLAGS = -Wall -Wextra -Werror

SRC = main.c coder_routine.c data_init.c sim_init.c threads_creat.c heap.c time.c monitor.c  



OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	cc $(CFLAGS) -o $(NAME) $(OBJ)


%.o: %.c
	cc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re

.SECONDARY: $(OBJ)