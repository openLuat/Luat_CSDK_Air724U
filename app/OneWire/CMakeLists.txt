
target_include_directories(${target} PUBLIC ./)
target_include_directories(${target} PUBLIC dht11)
target_include_directories(${target} PUBLIC ./ds18b20)

target_sources(${target} PUBLIC
    ./OneWire.c
    dht11/dht11.c
    ds18b20/ds18b20.c
)