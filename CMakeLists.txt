 cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(evl_example C)  # 注意这里指定使用 C 语言

set(TASK_NAME evl_main)
# 添加可执行文件
add_executable(${TASK_NAME} src/evl_main.c)

# 设置包含目录
target_include_directories(${TASK_NAME} 
    PUBLIC /usr/local/include
)

# 设置库文件目录
target_link_directories(${TASK_NAME}
    PUBLIC /usr/local/lib
)

# 设置需要链接的库
target_link_libraries(${TASK_NAME}
    evl
    pthread
    rt
)