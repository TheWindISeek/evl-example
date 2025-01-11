 cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(evl_example C)  # 注意这里指定使用 C 语言

set(EVL_MAIN evl_main)
# 添加可执行文件
add_executable(${EVL_MAIN} src/evl_main.c)

# 设置包含目录
target_include_directories(${EVL_MAIN} 
    PUBLIC /usr/local/include
)

# 设置库文件目录
target_link_directories(${EVL_MAIN}
    PUBLIC /usr/local/lib
)

# 设置需要链接的库
target_link_libraries(${EVL_MAIN}
    evl
    pthread
    rt
)

set(EVL_TIMER timer)
add_executable(${EVL_TIMER} src/timer.c)

target_include_directories(${EVL_TIMER} 
    PUBLIC /usr/local/include
)

target_link_directories(${EVL_TIMER}
    PUBLIC /usr/local/lib
)
target_link_libraries(${EVL_TIMER}
    evl
    pthread
    rt
)