if(ENABLE_SSL)
  # Generate SSL certs and keys when needed
  set(SSL_CONFIGS ca.crt ca.key ca.txt redis.crt redis.key client.crt client.key)
  add_custom_command(
    OUTPUT ${SSL_CONFIGS}
    COMMAND openssl genrsa -out ca.key 4096
    COMMAND openssl req -x509 -new -nodes -sha256 -key ca.key -days 3650 -subj '/CN=Redis Test CA' -out ca.crt
    COMMAND openssl genrsa -out redis.key 2048
    COMMAND openssl req -new -sha256 -key redis.key -subj '/CN=Redis Server Test Cert' | openssl x509 -req -sha256 -CA ca.crt -CAkey ca.key -CAserial ca.txt -CAcreateserial -days 365 -out redis.crt
    COMMAND openssl genrsa -out client.key 2048
    COMMAND openssl req -new -sha256 -key client.key -subj '/CN=Redis Client Test Cert' | openssl x509 -req -sha256 -CA ca.crt -CAkey ca.key -CAserial ca.txt -CAcreateserial -days 365 -out client.crt
    )
  add_custom_target(generate_tls_configs DEPENDS ${SSL_CONFIGS})

  set(SSL_LIBRARY hiredis_ssl)
endif()


find_library(EVENT_LIBRARY event HINTS /usr/lib/x86_64-linux-gnu)

if(MSVC)
  # MS Visual: Suppress warnings
  add_compile_options("/wd 4267" "/wd 4244")
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Executable: IPv4
add_executable(example_ipv4 main.c)
target_link_libraries(example_ipv4 hiredis_cluster hiredis ${SSL_LIBRARY})
add_test(NAME example_ipv4 COMMAND "$<TARGET_FILE:example_ipv4>")

# Executable: IPv6
add_executable(example_ipv6 main_ipv6.c)
target_link_libraries(example_ipv6 hiredis_cluster hiredis ${SSL_LIBRARY})
add_test(NAME example_ipv6 COMMAND "$<TARGET_FILE:example_ipv6>")

# Executable: async
add_executable(example_async main_async.c)
target_link_libraries(example_async hiredis_cluster hiredis ${SSL_LIBRARY} ${EVENT_LIBRARY})
add_test(NAME example_async COMMAND "$<TARGET_FILE:example_async>")

add_executable(ct_commands ct_commands.c)
target_link_libraries(ct_commands hiredis_cluster hiredis ${SSL_LIBRARY})
add_test(NAME ct_commands COMMAND "$<TARGET_FILE:ct_commands>")

if(ENABLE_SSL)
  # Executable: tls
  add_executable(example_tls main_tls.c)
  target_link_libraries(example_tls hiredis_cluster hiredis ${SSL_LIBRARY})
  add_dependencies(example_tls generate_tls_configs)
  add_test(NAME example_tls COMMAND "$<TARGET_FILE:example_tls>")

  # Executable: async tls
  add_executable(example_async_tls main_async_tls.c)
  target_link_libraries(example_async_tls hiredis_cluster hiredis ${SSL_LIBRARY} ${EVENT_LIBRARY})
  add_dependencies(example_async_tls generate_tls_configs)
  add_test(NAME example_async_tls COMMAND "$<TARGET_FILE:example_async_tls>")
endif()
