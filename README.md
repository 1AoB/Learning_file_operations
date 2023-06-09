# Learning_file_operations
文件操作的学习

# 1. 重启网络

```bash
ifdown eth0
ifup eth0
```

# 2. 查看进程号

```bash
ps -ef | grep 进程名
比如 gcc main.c - o main,这个变成生成的可执行文件main就是进程名
```

![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/8a84e4fc-1bd0-42c9-9b3d-f6e4d63a4d24)

# 3. man

第二卷是系统调用

第三卷是库调用

# 4. 通过lseek,读取到file1这个文件的大小
```c
//file1 size
ret = lseek(file1,0,SEEK_END);
```
# 5. Linux的运行空间 = 内核空间 + 用户空间

# 6. I/O的两种访问方式
## 标准文件访问方式
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/75e593cc-1e7f-4606-b736-2a151fe56ccc)

## 直接IO(open函数中增加O_DIRECT)
直接IO 绕过缓存，不会出现write成功数据丢失的情况，血案不再重演.

注意：直接I/O的缺点就是如果访问的数据不在应用程序缓存中，
那么每次数据都会直接从磁盘进行加载，这种直接加载会非常缓慢！
通常直接I/O跟异步I/O结合使用会得到较好的性能。
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/76b17439-5252-4ea5-aa6d-5281fa02f37e)

直接IO需要进行的三个步骤:
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/f5f40977-e6ad-43ab-8d98-9feff050face)

## 注意！
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/f70e7a6b-befc-4876-b044-46a903845ebd)

O_DIRECT是:跳过高速页缓存,直接存到物理磁盘 **缺点:每次从磁盘中加载非常慢**

O_SYNC是:不跳过高速页缓存,到了高速页缓存后,马不停蹄的存到物理磁盘,才返回写入成功. **缺点:这种数据有保证,但是效率并不高**

以上两个都不写:不跳过高速页缓存,到了高速页缓存后,此时返回写入成功并在高速页缓存停留一会之后,才存入到物理磁盘.(如果在停留的这会时间段内,服务器宕机,数据就会丢失无法找回)


# 7.O_DIRECT不用,O_SYNC也不用,那应该用什么?

缓存同步可以保证**我们想啥时候把数据从高速页缓存同步到磁盘,就啥时候把数据从高速页缓存同步到磁盘,完全有我们自己来确定同步到磁盘的时机!**
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/038bb6a1-8652-442c-a244-853c5bcc69e4)



- sync - 将所有修改过的块缓冲区排入写队列，然后就返回，它并**不等待实际写磁盘操作结束**。**(并不能保证数据有效,不是我们想要的!!!)**
- fsync - 将fd对应文件的块缓冲区**立即写入磁盘**，并等待实际写磁盘操作结束返回。**(很明显,这个才是我们想要的)**
- fdatasync - 类似fsync,但**只影响文件的数据部分(也就是说它只同步文件内容)**。而除数据外，fsync 还会同步更新文件属性。

# 8.Linux 文件IO流程图
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/64b3967b-7527-46c8-a494-a0deb0c8fa5e)

# 9. 文件锁  fcntl函数

1) 并发对文件I/O操作的影响
      
      解决办法？使用文件锁fcntl函数
      
2) 文件锁

- 用法：man 2 fcntl

- 头文件：


```c
#include <unistd.h>
#include <fcntl.h>
```

- 函数定义：int fcntl(int fd, int cmd, ... /* arg */ );

   -  参数： cmd 取值  F_GETLK,   F_SETLK 和 F_SETLKW ,分别表示获取锁、设置锁和同步设置锁.

   **F_SETLK是如果没锁就设置锁,有锁也不等,直接函数调用(即不阻塞)**

   **F_SETLKW是如果没锁就设置锁,有锁就一直等到对方解锁,然后再上锁(即阻塞)**

- 文件锁的表示

1)struct flock

```c
// struct flock 结构体说明

struct flock {
    short l_type;  /*F_RDLCK, F_WRLCK, or F_UNLCK */
    off_t l_start;  /*offset in bytes, relative to l_whence */
    short l_whence;  /*SEEK_SET, SEEK_CUR, or SEEK_END */ //分别表示开头,当前,结尾
    off_t l_len;   /*length, in bytes; 0 means lock to EOF */
    pid_t l_pid;   /*returned with F_GETLK */
};

l_type:  第一个成员是加锁的类型：只读锁，读写锁，或是解锁。
l_start和l_whence: 用来指明加锁部分的开始位置。
l_len: 是加锁的长度。
l_pid: 是加锁进程的进程id。
```
2)举例： 

我们现在需要把一个文件的前三个字节加读锁，则该结构体的l_type=F_RDLCK, l_start=0,
l_whence=SEEK_SET,  l_len=3,  **l_pid不需要指定**，然后调用fcntl函数时，
cmd参数使F_SETLK.
```c
//
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME 	"test.txt"  //要操作的文件名
	   
int flock_set(int fd, int type)
{
	printf("pid=%d into...\n", getpid());//通过getpid()获取当前进程号
	
	struct flock flock;//这个结构体已在#include <fcntl.h>中定义,直接用就可以
	

	fcntl(fd, F_GETLK, &flock);//先通过F_GETLK获取锁的状态

	if (flock.l_type != F_UNLCK) {
		if (flock.l_type == F_RDLCK) {
			printf("flock has been set to read lock by %d\n", flock.l_pid);
		} else if (flock.l_type == F_WRLCK) {
			printf("flock has been set to write lock by %d\n", flock.l_pid);
		}
	}

  	flock.l_type = type;
	flock.l_whence = SEEK_SET;//从文件开头上锁
	flock.l_start = 0;//偏移0个字节
	flock.l_len = 0;//0 means lock to EOF,即:0表示上锁的部分一直持续到文件尾部   
	flock.l_pid = -1;
  
	if (fcntl(fd, F_SETLKW, &flock) < 0) {//设置阻塞同步锁,锁的类型已经包含在flock.l_type中
		printf("set lock failed!\n");
		return -1;
	}	
  switch (flock.l_type) {//查看锁的类型
	case F_RDLCK:
		printf("read lock is set by %d\n", getpid());//读,也叫共享锁
		break;
	case F_WRLCK:
		printf("write lock is set by %d\n", getpid());//写,也叫排他锁;(读读都可上锁;先读后写,写锁不能上;先写后读,读锁不能上;写写,第二个写锁不能上)
		break;
	case F_UNLCK:
		printf("lock is released by %d\n", getpid());//释放
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

	fd = open(FILE_NAME, O_RDWR|O_CREAT, 0666);//可读可写,没有就创建
	if (fd < 0) {
		printf("open file %s failed!\n", FILE_NAME);
	}

	//flock_set(fd, F_WRLCK);//设置写锁
	flock_set(fd, F_RDLCK);//设置读锁
	getchar();
	flock_set(fd, F_UNLCK);//释放锁
	getchar();

	close(fd);
	return 0;
}
```
