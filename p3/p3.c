#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

const char *name = "OS";
const char *roger = "Roger Roger";
const int SIZE = 4096;

int compare_files(const char *input_file, const char *output_file);

int main(int argc, char *argv[])
{
    char content[SIZE];

    FILE *input_file = fopen(argv[1], "r");
    int input_size = fread(content, 1, SIZE, input_file);
    fclose(input_file);

    int fd1[2], fd2[2];
    if (pipe(fd1) == -1 || pipe(fd2) == -1)
    {
        perror("Pipe creation failed");
        exit(1);
    }

    if (fork() == 0)
    { // Child process
        close(fd1[1]);
        char buffer[SIZE];
        read(fd1[0], buffer, input_size);

        int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, input_size);
        char *shm_ptr = mmap(0, input_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        memcpy(shm_ptr, buffer, input_size);
        munmap(shm_ptr, input_size);
        close(shm_fd);

        write(fd2[1], roger, strlen(roger));
        exit(0);
    }

    close(fd1[0]);
    write(fd1[1], content, input_size);

    char ack_buffer[strlen(roger)];
    read(fd2[0], ack_buffer, strlen(roger));

    if (strcmp(ack_buffer, roger) == 0)
    {
        if (fork() == 0)
        { // Grandchild process
            int shm_fd = shm_open(name, O_RDONLY, 0666);
            char *shm_ptr = mmap(0, input_size, PROT_READ, MAP_SHARED, shm_fd, 0);

            FILE *output_file = fopen(argv[2], "w");
            fwrite(shm_ptr, 1, input_size, output_file);
            fclose(output_file);
            munmap(shm_ptr, input_size);
            close(shm_fd);

            exit(0);
        }
        wait(NULL);
        if (compare_files(argv[1], argv[2]) == 0)
        {
            exit(0);
        }
    }
    return 1;
}

int compare_files(const char *input_file, const char *output_file)
{
    FILE *f1 = fopen(input_file, "r");
    FILE *f2 = fopen(output_file, "r");
    int diff = (f1 && f2) ? 0 : 1; // assume different if files can't be opened

    char ch1, ch2;
    while (!diff && (ch1 = fgetc(f1)) != EOF && (ch2 = fgetc(f2)) != EOF)
    {
        if (ch1 != ch2)
        {
            diff = 1;
        }
    }

    if (f1)
        fclose(f1);
    if (f2)
        fclose(f2);
    return diff;
}
