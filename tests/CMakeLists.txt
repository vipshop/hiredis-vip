# Generate SSL certs and keys when needed
set(SSL_CONFIGS ca.crt ca.key ca.txt redis.crt redis.key)
add_custom_command(
  OUTPUT ${SSL_CONFIGS}
  COMMAND openssl genrsa -out ca.key 4096
  COMMAND openssl req -x509 -new -nodes -sha256 -key ca.key -days 3650 -subj '/CN=Redis Test CA' -out ca.crt
  COMMAND openssl genrsa -out redis.key 2048
  COMMAND openssl req -new -sha256 -key redis.key -subj '/CN=Redis Test Cert' | openssl x509 -req -sha256 -CA ca.crt -CAkey ca.key -CAserial ca.txt -CAcreateserial -days 365 -out redis.crt
)
add_custom_target(generate_tls_configs DEPENDS ${SSL_CONFIGS})

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
target_link_libraries(example_tls hiredis_vip hiredis hiredis_ssl)
add_dependencies(example_tls generate_tls_configs)
add_test(NAME example_tls COMMAND "$<TARGET_FILE:example_tls>")
