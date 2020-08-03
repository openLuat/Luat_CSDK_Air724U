
set(target lua_OneWire)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})



target_include_directories(${target} PUBLIC ./)
target_include_directories(${target} PUBLIC dht11)
target_include_directories(${target} PUBLIC ./ds18b20)
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)


target_sources(${target} PUBLIC
    ./OneWire.c
    dht11/dht11.c
    ds18b20/ds18b20.c
)

