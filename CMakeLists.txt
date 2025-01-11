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