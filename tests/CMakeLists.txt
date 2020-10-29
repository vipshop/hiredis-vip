# Executable: IPv4
add_executable(example_ipv4 main.c)
target_link_libraries(example_ipv4 hiredis_vip hiredis)
add_test(NAME example_ipv4 COMMAND "$<TARGET_FILE:example_ipv4>")

# Executable: IPv6
add_executable(example_ipv6 main_ipv6.c)
target_link_libraries(example_ipv6 hiredis_vip hiredis)
add_test(NAME example_ipv6 COMMAND "$<TARGET_FILE:example_ipv6>")

# Executable: async
add_executable(example_async main_async.c)
find_library(EVENT_LIBRARY event HINTS /usr/lib/x86_64-linux-gnu)
target_link_libraries(example_async hiredis_vip hiredis ${EVENT_LIBRARY})
add_test(NAME example_async COMMAND "$<TARGET_FILE:example_async>")

# Executable: tls
add_executable(example_tls main_tls.c)
target_link_libraries(example_tls hiredis_vip hiredis)
add_test(NAME example_tls COMMAND "$<TARGET_FILE:example_tls>")
