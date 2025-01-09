// #include <evl/thread.h>
// #include <evl/clock.h>
// #include <evl/sys.h>
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