cmake_minimum_required(VERSION 2.4)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    
    include(CheckIncludeFiles)
    
    #CHECK_INCLUDE_FILES(cpufreq.h HAVE_CPUFREQ_H)
    CHECK_INCLUDE_FILES(pcap.h HAVE_PCAP_H)
    
    #if (NOT "${HAVE_CPUFREQ_H}" STREQUAL "1" )
    #        message(FATAL_ERROR "libcpufreq-dev package not installed...")
    #endif (NOT "${HAVE_CPUFREQ_H}" STREQUAL "1" )
    
    if (NOT ${HAVE_PCAP_H})
            message(FATAL_ERROR "libpcap-dev package not installed...")
    endif(NOT ${HAVE_PCAP_H})
    
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(.)

add_definitions(-DARCH_i386 -DMORE_USE_TR1_SHARED_PTR -DMORE_USE_GNU_MUTEX)


# To compile with icpc use the following CXXFLAGS: -gcc-name=gcc-4.2 -gxx-name=g++-4.2
# that solves the issue __builtin_fpclassify
#

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

file(GLOB testfiles "*.cc" )

foreach(testfile ${testfiles})
    
    GET_FILENAME_COMPONENT(testname ${testfile} NAME_WE) # remove the '.cpp' part
    
    message (STATUS "Building ${testname}...")

    add_executable(${testname} ${testfile})
    
    target_link_libraries(${testname}  ${TEST_LINK_LIBS})
    
endforeach(testfile)

target_link_libraries(arch-cpu-test  -lcpufreq)
target_link_libraries(pcap-test -lpcap)
target_link_libraries(pcap-test-header -lpcap)
target_link_libraries(semaphore-test -pthread)
target_link_libraries(atomic-test -pthread)
target_link_libraries(pthread-test -pthread)
target_link_libraries(shared_queue-test -pthread)
target_link_libraries(refcnt-test -pthread)
target_link_libraries(timer-test -pthread -lrt)
target_link_libraries(tstreambuf-test -pthread)
target_link_libraries(spinlock-test -pthread)
target_link_libraries(tqueue-test -pthread)
target_link_libraries(cpufreq-test -lcpufreq)

endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
