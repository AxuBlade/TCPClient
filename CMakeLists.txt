project(tcpclient)

add_executable(tcpclient main.c)

install(TARGETS tcpclient RUNTIME DESTINATION bin)
