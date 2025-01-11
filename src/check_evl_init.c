#include <evl/evl.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int ret;

    ret = evl_init();
	printf("ret: %d\n", ret);
    if (ret < 0) {
        printf("failed to initialize EVL core\n");
        return -1;
    }

    printf("EVL core initialized successfully\n");
    return 0;
}