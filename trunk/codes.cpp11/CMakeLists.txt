cmake_minimum_required(VERSION 2.4)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    
    include(CheckIncludeFiles)
    
    CHECK_INCLUDE_FILES(pcap.h HAVE_PCAP_H)
    
    if (NOT ${HAVE_PCAP_H})
            message(FATAL_ERROR "libpcap-dev package not installed...")
    endif(NOT ${HAVE_PCAP_H})
    
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(.)

add_definitions(-D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wsign-conversion")
 
file(GLOB testfiles "*.cpp" )

foreach(testfile ${testfiles})
    
    GET_FILENAME_COMPONENT(testname ${testfile} NAME_WE) # remove the '.cpp' part
    
    message (STATUS "Building ${testname}...")

    add_executable(${testname} ${testfile})
    
    target_link_libraries(${testname}  ${TEST_LINK_LIBS})
    
endforeach(testfile)

target_link_libraries(cpufreq-test -lcpufreq)
target_link_libraries(pcap-test -lpcap)
target_link_libraries(pcap-test-header -lpcap)
target_link_libraries(spinlock-test -pthread)
target_link_libraries(logger-test -pthread)
