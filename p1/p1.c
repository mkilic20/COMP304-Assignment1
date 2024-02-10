#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

char *get_time()
{
    static char buffer[80];
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);

    return buffer;
}

void rec(int n, int level)
{
    if (n > 0)
    {
        int val = fork();
        if (val == 0)
        {
            wait(NULL);
            char *time = get_time();
            for (int i = 0; i < 3; i++)
            {
                printf("Process ID: %d, Parent ID: %d, level: %d, time: %s\n", getpid(), getppid(), level, time);
                sleep(1);
            }
            rec(n - 1, level + 1);
            exit(0);
        }
        else
        {
            wait(NULL);
            /*rec(n - 1, level);*/
        }
    }

    return;
}

int main(int argc, char *argv[])
{
    printf("Main Process ID: %d, level: 0, time: %s\n", getpid(), get_time());
    int n = atoi(argv[1]);
    rec(n, 1);
    return 0;
}
