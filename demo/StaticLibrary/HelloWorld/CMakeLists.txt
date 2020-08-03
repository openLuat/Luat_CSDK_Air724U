
include_directories(../../../api/include)
include_directories(./inc)
aux_source_directory(./src DIR_SRCS)
add_library(HelloWorld STATIC ${DIR_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/demo/${BUILD_TARGET}/HelloWorld/lib)
