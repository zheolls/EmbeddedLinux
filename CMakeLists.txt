# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("LED")

# 包含子项目。
add_subdirectory ("test_led")
add_subdirectory("tcp")

#Linux支持头文件
include_directories("/usr/src/linux-2.6.24.4/include")
include_directories("/usr/src/linux-2.6.24.4/arch")

set(KERNEL_DIR "/usr/src/linux-2.6.24.4")


function(compile_module obj)
  set(TARGET_NAME ${obj})
  add_custom_target(${TARGET_NAME} ALL cp -f ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_BINARY_DIR}/
  		    COMMAND echo "compiling module ${obj}.ko..."
  		    )
  list(LENGTH ARGN argn_len)
  set(i 0)
  set(depend_objlist "")
  while( i LESS ${argn_len})  
	list(GET ARGN ${i} argn_value)  
	set(depend_objlist "${depend_objlist} ${argn_value}.o") 
	math(EXPR i "${i} + 1")  
  endwhile() 
  
  add_custom_command(TARGET ${TARGET_NAME}
		     POST_BUILD
		     COMMAND echo "obj-m := ${obj}.o" > ${CMAKE_CURRENT_BINARY_DIR}/Makefile
		     COMMAND echo "${obj}-objs:=${depend_objlist}" >>${CMAKE_CURRENT_BINARY_DIR}/Makefile
		     COMMAND make -C ${KERNEL_DIR} M=${CMAKE_CURRENT_BINARY_DIR} modules
			 COMMAND rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions

		     )
endfunction()

compile_module(led)
