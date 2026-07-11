#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *tester()
{
    int i;

    i = 0;
    while (i < 100)
    {
        write(1, "1\n", 2);
        ++i;
    }
    return (NULL);
}
void *tester2()
{
    int i;

    i = 0;
    while (i < 100)
    {
        write(1, "2\n", 2);
        ++i;
    }
    return (NULL);
}

int main()
{
    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, NULL, tester, NULL);
    pthread_create(&thread2, NULL, tester2, NULL);
    // usleep(3000);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return (0);
}