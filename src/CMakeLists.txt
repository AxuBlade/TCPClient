include_directories("${PROJECT_SOURCE_DIR}/src")


add_executable(tcpclient main.c connection_handling.c parsers.c)
add_subdirectory(commands)
add_subdirectory(semaphores)
add_dependencies (tcpclient commands)
add_dependencies (tcpclient semaphores)
target_link_libraries(tcpclient commands)
target_link_libraries(tcpclient semaphores)
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/client DESTINATION bin)
