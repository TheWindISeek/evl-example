项目地址在https://github.com/TheWindISeek/evl-example.git

# 前置工作

## 确认EVL内核开启

方法1

```shell
# 通过查看系统日志的内容来确认evl是否开启
dmesg | grep EVL
```

方法2

这种方法需要提前将单元测试下载到/usr/local/下

```shell
# 通过运行evl提供的单元测试来判断内核是否开启
export EVL_TESTDIR=/usr/local/libexec/evl/tests
sudo -E /usr/local/libexec/evl/evl-test
```

## 确认evl头文件的位置

我的evl头文件放在了/usr/local/include下

输出示例如下所示

```shell
neu@neu-pi:~$ # 确认evl是否开启
dmesg | grep EVL
[    1.270031] IRQ pipeline: high-priority EVL stage added.
[    1.273413] EVL: core started [DEBUG]
neu@neu-pi:~$ ls /usr/local/include/evl/
atomic.h     compat.h       devices  factory-abi.h  heap.h         mutex.h           observable.h  proxy-abi.h  rwlock.h     sem.h          syscall.h     thread.h  types.h
clock-abi.h  compiler.h     event.h  fcntl.h        list.h         net               poll-abi.h    proxy.h      sched-abi.h  signal.h       sys.h         timer.h   xbuf-abi.h
clock.h      control-abi.h  evl.h    flags.h        monitor-abi.h  observable-abi.h  poll.h        ring_ptr.h   sched.h      syscall-abi.h  thread-abi.h  trace.h   xbuf.h


```

# 编写代码

## 只有evl初始化的代码

下面的这个代码只调用了evl_init的接口，并根据ret来进行输出提示信息。

干扰的因素较少，可以用来判断头文件是否能找到以及evl_init是否能够成功。

```cpp
#include <evl/evl.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int ret;

    ret = evl_init();
	printf("ret: %d\n", ret);
    if (ret < 0) {
        printf("failed to initialize EVL core\n");
    }

    printf("EVL core initialized successfully\n");
    return 0;
}
```

用于编译上述代码的shell指令

```shell
gcc check_evl_init.c -o check_evl_init -I/usr/local/include -L/usr/local/lib -levl -lpthread -lrt
```

运行结果如下所示

```shell
neu@neu-pi:~/Codes/evl-projects/src$ gcc check_evl_init.c -o check_evl_init -I/usr/local/include -L/usr/local/lib -levl -lpthread -lrt
neu@neu-pi:~/Codes/evl-projects/src$ sudo ./check_evl_init 
[sudo] password for neu: 
ret: 0
EVL core initialized successfully
neu@neu-pi:~/Codes/evl-projects/src$ 
```

注意，运行evl相关程序是需要root权限的。

否则就会像下面这样，返回-13

```shell
neu@neu-pi:~/Codes/evl-projects/src$ gcc check_evl_init.c -o check_evl_init -I/usr/local/include -L/usr/local/lib -levl -lpthread -lrt
neu@neu-pi:~/Codes/evl-projects/src$ ./check_evl_init 
ret: -13
failed to initialize EVL core
```

## 创建timer并等待timer执行200次的代码

```cpp
#include <evl/evl.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <evl/thread.h>
#include <evl/timer.h>
#include <evl/clock.h>
#include <evl/syscall.h>

// 辅助函数：添加纳秒到 timespec
static void timespec_add_ns(struct timespec *ts, const struct timespec *base, unsigned long long ns)
{
    ts->tv_sec = base->tv_sec + ns / 1000000000ULL;
    ts->tv_nsec = base->tv_nsec + ns % 1000000000ULL;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_nsec -= 1000000000;
        ts->tv_sec++;
    }
}

int main(int argc, char *argv[])
{
    struct itimerspec value, ovalue;
    struct sched_param param;
    struct timespec now;
    int tmfd, ret, n;
    __u64 ticks;

    // 设置线程为 SCHED_FIFO 策略
    param.sched_priority = 1;
    ret = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    if (ret != 0) {
        error(1, ret, "无法设置调度策略");
    }

    // 附加到 EVL 核心
    tmfd = evl_attach_self("periodic-timer:%d", getpid());
    if (tmfd < 0) {
        error(1, -tmfd, "无法附加到 EVL 核心");
    }
    printf("成功附加到 EVL 核心\n");

    // 切换到实时模式
    ret = evl_switch_oob();
    if (ret < 0) {
        error(1, -ret, "无法切换到实时模式");
    }

    // 创建定时器
    tmfd = evl_new_timer(EVL_CLOCK_MONOTONIC);
    if (tmfd < 0) {
        error(1, -tmfd, "无法创建定时器");
    }
    printf("成功创建定时器\n");

    // 读取当前时间
    ret = evl_read_clock(EVL_CLOCK_MONOTONIC, &now);
    if (ret < 0) {
        error(1, -ret, "无法读取时钟");
    }

    // 设置定时器参数
    // 初始延迟1秒
    timespec_add_ns(&value.it_value, &now, 1000000000ULL);
    // 周期为10ms
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 10000000;

    // 启动定时器
    ret = evl_set_timer(tmfd, &value, &ovalue);
    if (ret < 0) {
        error(1, -ret, "无法设置定时器");
    }
    printf("定时器已启动，周期为10ms\n");

    // 等待200个周期
    for (n = 0; n < 200; n++) {
        ret = oob_read(tmfd, &ticks, sizeof(ticks));
        if (ret < 0) {
            error(1, -ret, "读取定时器失败");
        }
        printf("第 %d 次触发，ticks=%llu\n", n + 1, ticks);
    }

    // 停止定时器
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;
    ret = evl_set_timer(tmfd, &value, &ovalue);
    if (ret < 0) {
        error(1, -ret, "无法停止定时器");
    }
    printf("定时器已停止\n");

    // 关闭定时器
    close(tmfd);

    return 0;
}
```

编译其的指令为

```shell
gcc check_evl_timer.c -o check_evl_timer -I/usr/local/include -L/usr/local/lib -levl -lpthread -lrt
```

正常输出为

```shell
neu@neu-pi:~/Codes/evl-projects/src$ sudo ./check_evl_timer 
成功附加到 EVL 核心
成功创建定时器
定时器已启动，周期为10ms
第 1 次触发，ticks=1
...
第 200 次触发，ticks=1
定时器已停止
```

# 使用CMakeLists.txt进行构建

每次使用gcc进行编译过于麻烦，于是我提取了gcc的编译参数，并编写了CMakeLists.txt。

```cmake
 cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(evl_example C)  # 注意这里指定使用 C 语言

set(CHECK_EVL_INIT check_evl_init)
# 添加可执行文件
add_executable(${CHECK_EVL_INIT} src/check_evl_init.c)

# 设置包含目录
target_include_directories(${CHECK_EVL_INIT} 
    PUBLIC /usr/local/include
)

# 设置库文件目录
target_link_directories(${CHECK_EVL_INIT}
    PUBLIC /usr/local/lib
)

# 设置需要链接的库
target_link_libraries(${CHECK_EVL_INIT}
    evl
    pthread
    rt
)

set(CHECK_EVL_TIMER check_evl_timer)  
add_executable(${CHECK_EVL_TIMER} src/check_evl_timer.c)

target_include_directories(${CHECK_EVL_TIMER} 
    PUBLIC /usr/local/include
)

target_link_directories(${CHECK_EVL_TIMER}
    PUBLIC /usr/local/lib
)
target_link_libraries(${CHECK_EVL_TIMER}
    evl
    pthread
    rt
)
```

如果想要添加新的源代码进行编译，将下述的check_EVL_EXAMPLE修改为自己的例子名称，并将add_executable中src下的代码修改为自己的代码即可

```cmake
set(CHECK_EVL_EXAMPLE check_evl_example)  
add_executable(${CHECK_EVL_EXAMPLE} src/check_evl_example.c)

target_include_directories(${CHECK_EVL_EXAMPLE} 
    PUBLIC /usr/local/include
)

target_link_directories(${CHECK_EVL_EXAMPLE}
    PUBLIC /usr/local/lib
)
target_link_libraries(${CHECK_EVL_EXAMPLE}
    evl
    pthread
    rt
)
```

具体的编译指令为

```shell
rm -rf build
mkdir build
cd build
cmake ..
make
# 这一步可以查看到自己编译的输出
ls
```