file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-hash-acc
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-hash-acc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-hash-acc
        main.cpp)

target_link_libraries(riscv-vp-hash-acc rv32 platform-hash-acc platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
