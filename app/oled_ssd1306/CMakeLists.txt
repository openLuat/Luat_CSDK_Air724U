target_include_directories(${target} PUBLIC ../../api/include)
target_include_directories(${target} PUBLIC inc)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(src DIR_SRCS)

target_sources(${target} PUBLIC ${DIR_SRCS})