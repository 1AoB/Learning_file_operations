//
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "test.txt" // 要操作的文件名

int flock_set(int fd, int type)
{
    printf("pid=%d into...\n", getpid()); // 通过getpid()获取当前进程号

    struct flock flock; // 这个结构体已在#include <fcntl.h>中定义,直接用就可以

    fcntl(fd, F_GETLK, &flock); // 先通过F_GETLK获取锁的状态

    if (flock.l_type != F_UNLCK)
    {
        if (flock.l_type == F_RDLCK)
        {
            printf("flock has been set to read lock by %d\n", flock.l_pid);
        }
        else if (flock.l_type == F_WRLCK)
        {
            printf("flock has been set to write lock by %d\n", flock.l_pid);
        }
    }

    flock.l_type = type;
    flock.l_whence = SEEK_SET; // 从文件开头上锁
    flock.l_start = 0;         // 偏移0个字节
    flock.l_len = 0;           // 0 means lock to EOF,即:0表示上锁的部分一直持续到文件尾部
    flock.l_pid = -1;

    if (fcntl(fd, F_SETLKW, &flock) < 0)
    { // 设置阻塞同步锁,锁的类型已经包含在flock.l_type中
        printf("set lock failed!\n");
        return -1;
    }
    switch (flock.l_type)
    { // 查看锁的类型
    case F_RDLCK:
        printf("read lock is set by %d\n", getpid()); // 读,也叫共享锁
        break;
    case F_WRLCK:
        printf("write lock is set by %d\n", getpid()); // 写,也叫排他锁;(读读都可上锁;先读后写,写锁不能上;先写后读,读锁不能上;写写,第二个写锁不能上)
        break;
    case F_UNLCK:
        printf("lock is released by %d\n", getpid()); // 释放
        break;
    default:
        break;
    }

    printf("pid=%d out.\n", getpid());
    return 0;
}

int main(void)
{
    int fd;

    fd = open(FILE_NAME, O_RDWR | O_CREAT, 0666); // 可读可写,没有就创建
    if (fd < 0)
    {
        printf("open file %s failed!\n", FILE_NAME);
    }

    flock_set(fd, F_WRLCK); // 设置写锁
    // flock_set(fd, F_RDLCK); // 设置读锁
    getchar();
    flock_set(fd, F_UNLCK); // 释放锁
    getchar();

    close(fd);
    return 0;
}