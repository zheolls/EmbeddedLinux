cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME "client")

add_custom_target(
	${TARGET_NAME} ALL qmake
	COMMAND qmake -project
	COMMAND make
)
