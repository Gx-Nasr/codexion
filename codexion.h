/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nel-adao <nel-adao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 10:47:15 by nel-adao          #+#    #+#             */
/*   Updated: 2026/07/15 14:15:26 by nel-adao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct s_sim t_sim;

typedef struct s_data
{
    int number_of_coders;
    int time_to_burnout;
    int time_to_compile;
    int time_to_debug;
    int time_to_refactor;
    int n_of_compiles_req;
    int dongle_cooldown;
    int is_edf;

}   t_data;

typedef struct s_request
{
    int id;
    long long preorety;

} t_request;

typedef struct s_dongle
{
    int is_taken;
    long long available_at;

    t_request queue[2];
    int size_queue;

    pthread_mutex_t d_mutex;
    pthread_cond_t d_cond;

    
} t_dongle;

typedef struct s_coder
{
    t_sim *sim;
    int             id;
    int             compile_count;
    long long            last_compile_t;
    pthread_t       thread;
    pthread_mutex_t c_mutex;
    t_dongle *left_dongle;
    t_dongle *right_dongle;

}   t_coder;

typedef struct s_sim
{
    t_data data;
    t_coder *coders;
    t_dongle *dongles;
    pthread_mutex_t s_mutex;
    long long start_time;
    int is_finished;
    int start;

} t_sim;



int data_init(char **av, t_data *data);
int sim_init(t_sim *sim, t_data *data);
void *coder_routine(void *arg);
int creat_threads(t_coder *coders, int n_coders);
long long get_time_ms();
void sort_edf(t_dongle *dongle);
void cond_wait_time(struct timespec *t_s, long long time);
void push_req(t_request *request, t_dongle *dongle, int is_edf);
void pop_req(t_dongle *dongle);
int end_checker(t_sim *sim);
int sleepr(long long sleep_time, t_sim *sim);
void cleaner(t_sim *sim, int i, int is_destroy);
