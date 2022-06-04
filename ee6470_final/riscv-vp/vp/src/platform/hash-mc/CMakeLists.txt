file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-hash-mc
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-hash-mc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-hash-mc
        main.cpp)

target_link_libraries(riscv-vp-hash-mc rv32 platform-hash-mc platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
