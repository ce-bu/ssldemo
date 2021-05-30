find_package(OpenSSL REQUIRED)

add_executable(client client.cpp)
add_executable(server server.cpp)

target_precompile_headers(client PRIVATE pch.h)
target_precompile_headers(server PRIVATE pch.h)

target_link_libraries(client
  PRIVATE
  ssl crypto
  glog
  )

target_link_libraries(server
  PRIVATE
  ssl crypto
  glog
  pthread
  )
