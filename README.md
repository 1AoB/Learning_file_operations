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

## 直接IO
直接IO 绕过缓存，不会出现write成功数据丢失的情况，血案不再重演.

注意：直接I/O的缺点就是如果访问的数据不在应用程序缓存中，
那么每次数据都会直接从磁盘进行加载，这种直接加载会非常缓慢！
通常直接I/O跟异步I/O结合使用会得到较好的性能。
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/76b17439-5252-4ea5-aa6d-5281fa02f37e)

直接IO需要进行的三个步骤:
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/f5f40977-e6ad-43ab-8d98-9feff050face)

# 注意！
![image](https://github.com/1AoB/Learning_file_operations/assets/78208268/f70e7a6b-befc-4876-b044-46a903845ebd)

O_DIRECT是:跳过高速页缓存,直接存到物理磁盘

O_SYNC是:不跳过高速页缓存,到了高速页缓存后,马不停蹄的存到物理磁盘,才返回写入成功.

以上两个都不写:不跳过高速页缓存,到了高速页缓存后,此时返回写入成功并在高速页缓存停留一会之后,才存入到物理磁盘.(如果在停留的这会时间段内,服务器宕机,数据就会丢失无法找回)



