#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int M = atoi(argv[1]);
    int N = atoi(argv[2]);

    if (M < N)
    {
        fprintf(stderr, "M is less than N");
        exit(1);
    }

    int number_list[M];
    srand(time(NULL));
    for (int i = 0; i < M; i++)
    {
        number_list[i] = rand() % 256;
    }

    int fd[N][2];
    for (int i = 0; i < N; i++)
    {
        if (pipe(fd[i]) == -1)
        {
            fprintf(stderr, "Pipe failed");
            exit(1);
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Fork failed");
            exit(1);
        }
        if (pid == 0)
        {
            close(fd[i][0]); // close read pipe
            int start = i * (M / N);
            int end;
            if (i == N - 1)
            {
                end = M;
            }
            else
            {
                end = start + (M / N);
            }

            int child_max = -1;
            for (int j = start; j < end; j++)
            {
                if (number_list[j] > child_max)
                {
                    child_max = number_list[j];
                }
            }

            write(fd[i][1], &child_max, 4);
            close(fd[i][1]); // close write pipe
            exit(0);
        }
        close(fd[i][1]); // close write pipe
    }

    int global_max = -1;
    for (int child_num = 0; child_num < N; child_num++)
    {
        int local_max;
        read(fd[child_num][0], &local_max, 4);
        if (local_max > global_max)
        {
            global_max = local_max;
        }
        close(fd[child_num][0]); // close read pipe
    }

    printf("%d\n", global_max);
    return 0;
}
