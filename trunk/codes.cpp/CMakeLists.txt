cmake_minimum_required(VERSION 2.4)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    
    include(CheckIncludeFiles)
    
    CHECK_INCLUDE_FILES(cpufreq.h HAVE_CPUFREQ_H)
    CHECK_INCLUDE_FILES(pcap.h HAVE_PCAP_H)
    
    if (NOT "${HAVE_CPUFREQ_H}" STREQUAL "1" )
            message(FATAL_ERROR "libcpufreq-dev package not installed...")
    endif (NOT "${HAVE_CPUFREQ_H}" STREQUAL "1" )
    
    if (NOT ${HAVE_PCAP_H})
            message(FATAL_ERROR "libpcap-dev package not installed...")
    endif(NOT ${HAVE_PCAP_H})
    
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(.)

add_definitions(-DARCH_i386 -DMORE_USE_TR1_SHARED_PTR -DMORE_USE_GNU_MUTEX)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
add_executable(CTdispatcher CTdispatcher.cc)
add_executable(any-test any-test.cc)
add_executable(algorithm-test algorithm-test.cc)
add_executable(async_fd-test async_fd-test.cc)
add_executable(atomic_test atomic-test.cc)
add_executable(atomicio_test atomicio-test.cc)
add_executable(atomicity-test atomicity-test.cc)
add_executable(arch-cpu-test arch-cpu-test.cc )
add_executable(buffer-test buffer-test.cc)
add_executable(cache-test cache-test.cc)
add_executable(collector-test collector-test.cc)
add_executable(colorful-test colorful-test.cc)
add_executable(concepts-test concepts-test.cc)
add_executable(cpp_idioms cpp_idioms.cc)
add_executable(cpp_test cpp_test.cc)
add_executable(cpuinfo-test cpuinfo-test.cc)
add_executable(cpufreq-test cpufreq-test.cc)
add_executable(cursor-test cursor-test.cc)
add_executable(cxa_demangle-test cxa_demangle-test.cc)
add_executable(delirium delirium.cc)
add_executable(dummystreambuf-test dummystreambuf-test.cc)
add_executable(enable_exception_test enable_exception-test.cc)
add_executable(endianless-test endianless-test.cc)
add_executable(enumap-test enumap-test.cc)
add_executable(errcode-test errcode-test.cc)
add_executable(error-test error-test.cc)
add_executable(exec-test exec-test.cc)
add_executable(ext_buffer_test ext_buffer-test.cc)
add_executable(exprtempl-test exprtempl-test.cc)
add_executable(factory-test factory-test.cc)
add_executable(fast_deque-test fast_deque-test.cc)
add_executable(flock-test flock-test.cc)
add_executable(fnv-test fnv-test.cc)
add_executable(format-test format-test.cc)
add_executable(functor-test functor-test.cc)
add_executable(getopt-test getopt-test.cc)
add_executable(gotopt-test gotopt-test.cc)
add_executable(gotopt-test-debug gotopt-test-debug.cc)
add_executable(heap-test heap-test.cc)
add_executable(hwtmpaflntavctr hwtmpaflntavctr.cc)
add_executable(inotify-test inotify-test.cc)
add_executable(iptables-test iptables-test.cc)
add_executable(itoa-test itoa-test.cc)
add_executable(iomanip-test iomanip-test.cc)
add_executable(kv_test kv-test.cc)
add_executable(lazy_object-test lazy_object-test.cc)
add_executable(lexical_cast-test lexical_cast-test.cc)
add_executable(lnistreambuf-test lnistreambuf-test.cc)
add_executable(logstream-test logstream-test.cc)
add_executable(matrix-test matrix-test.cc)
add_executable(md5-test md5-test.cc md5.cc)
add_executable(mmap_test mmap-test.cc)
add_executable(movable-test movable-test.cc)
add_executable(mutant-test mutant-test.cc)
add_executable(mutex_test mutex-test.cc)
add_executable(nullptr-test nullptr-test.cc)
add_executable(net-headers-test net-headers-test.cc)
add_executable(observer-test observer-test.cc)
add_executable(oracle-test oracle-test.cc)
add_executable(oracle-test-STL oracle-test-STL.cc)
add_executable(oracle-test-dynamic oracle-test-dynamic.cc)
add_executable(ostream_diverter-test ostream_diverter-test.cc)
add_executable(pcap-test pcap-test.cc)
add_executable(pcap-test-header pcap-test-header.cc)
add_executable(proxy-test proxy-test.cc)
add_executable(pthread-test pthread-test.cc)
add_executable(refcnt-test refcnt-test.cc)
add_executable(scoped_raii-test scoped_raii-test.cc)
add_executable(semaphore-test semaphore-test.cc)
add_executable(shared_ptr_test shared_ptr-test.cc)
add_executable(singleton-test singleton-test.cc)
add_executable(singleton_proc_test singleton_proc-test.cc)
add_executable(socket-test socket-test.cc)
add_executable(sprint2-test sprint2-test.cc)
add_executable(static_assert-test static_assert-test.cc)
add_executable(streamer-test streamer-test.cc)
add_executable(string-utils-test string-utils-test.cc)
add_executable(syslog-test syslog.cc syslog-test.cc)
add_executable(template_util-test template_util-test.cc)
add_executable(test-constness test-constness.cc)
add_executable(test-enable_if test-enable_if.cc)
add_executable(the-perfect-forwarder the-perfect-forwarder.cc)
add_executable(timer-test timer-test.cc)
add_executable(timeval-test timeval-test.cc)
add_executable(tqueue-test tqueue-test.cc)
add_executable(traits-info-test traits-info-test.cc)
add_executable(tspinlock-test tspinlock-test.cc)
add_executable(tstreambuf-test tstreambuf-test.cc)
add_executable(tuplarr-test tuplarr-test.cc)
add_executable(tuple-test tuple-test.cc)
add_executable(type_traits_test type_traits-test.cc)
add_executable(typelist-test typelist-test.cc)
add_executable(typemap-test typemap-test.cc)
add_executable(variant-test variant-test.cc)
add_executable(yaccp-test yacpp-test.cc)

target_link_libraries(arch-cpu-test -lcpufreq)
target_link_libraries(pcap-test -lpcap)
target_link_libraries(pcap-test-header -lpcap)
target_link_libraries(semaphore-test -pthread)
target_link_libraries(atomic_test -pthread)
target_link_libraries(fast_deque-test -pthread)
target_link_libraries(pthread-test -pthread)
target_link_libraries(refcnt-test -pthread)
target_link_libraries(timer-test -pthread -lrt)
target_link_libraries(tstreambuf-test -pthread)
target_link_libraries(tqueue-test -pthread)
target_link_libraries(tspinlock-test -pthread)
target_link_libraries(cpufreq-test -lcpufreq)

endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")