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