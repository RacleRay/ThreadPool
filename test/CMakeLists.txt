# # for each "test/x.cpp", generate target "x"
# file(GLOB_RECURSE all_tests *.cpp)
# foreach(v ${all_tests})
#     string(REGEX MATCH "test/.*" relative_path ${v})
#     # message(${relative_path})
#     string(REGEX REPLACE "test/" "" target_name ${relative_path})
#     string(REGEX REPLACE ".cpp" "" target_name ${target_name})

#     add_executable(${target_name} ${v})
# endforeach()


# # for each "test/x.c", generate target "x"
# file(GLOB_RECURSE all_tests *.c)
# foreach(v ${all_srcs})
#     string(REGEX MATCH "test/.*" relative_path ${v})
#     # message(${relative_path})
#     string(REGEX REPLACE "test/" "" target_name ${relative_path})
#     string(REGEX REPLACE ".c" "" target_name ${target_name})

#     add_executable(${target_name} ${v})
# endforeach()


add_executable(pooltest test_threadpool.cpp 
                ../src/supervisor.cpp 
                ../src/threadbatch.cpp 
                ../src/threadpool.cpp)


add_executable(supervisortest test_supervisor.cpp 
                ../src/supervisor.cpp 
                ../src/threadbatch.cpp 
                ../src/threadpool.cpp)


add_executable(threadbatch test_threadbatch.cpp
                ../src/supervisor.cpp 
                ../src/threadbatch.cpp 
                ../src/threadpool.cpp)
