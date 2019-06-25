cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME "client")

aux_source_directory(. DIR_SRCS)

add_custom_target(
	${TARGET_NAME} ALL cp -f ${CMAKE_CURRENT_SOURCE_DIR}/* ${CMAKE_CURRENT_BINARY_DIR}/
	COMMAND qmake -project
	COMMAND qmake 
	COMMAND make
	DEPENDS ${DIR_SRCS}
)
