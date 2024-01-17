find_package(Git)
if(GIT_EXECUTABLE)
	get_filename_component(WORKING_DIR ${SRC} DIRECTORY)
	execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:'%h' -n 1
					OUTPUT_VARIABLE GIT_REV
					WORKING_DIRECTORY ${WORKING_DIR}
					ERROR_QUIET)
	if ("${GIT_REV}" STREQUAL "")
		set(GIT_REV "N/A")
		set(GIT_DIFF "")
		set(GIT_TAG "N/A")
		set(GIT_BRANCH "N/A")
		set(BUILD_VERSION "N/A")
	else()
		execute_process(
			COMMAND ${GIT_EXECUTABLE} diff --quiet --exit-code
			WORKING_DIRECTORY ${WORKING_DIR}
			RESULT_VARIABLE GIT_DIFF)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} --exact-match --tags
			WORKING_DIRECTORY ${WORKING_DIR}
			OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
			WORKING_DIRECTORY ${WORKING_DIR}
			OUTPUT_VARIABLE GIT_BRANCH)

		string(STRIP "${GIT_REV}" GIT_REV)
		string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
		if(NOT (GIT_DIFF EQUAL "0"))
			set(GIT_DIFF "*")
		endif()
		string(STRIP "${GIT_DIFF}" GIT_DIFF)
		string(STRIP "${GIT_TAG}" GIT_TAG)
		string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
		if(GIT_TAG STREQUAL "")
			set(BUILD_VERSION "${GIT_BRANCH}-${GIT_REV}${GIT_DIFF}")
		else()
			set(BUILD_VERSION "${GIT_TAG}-${GIT_REV}${GIT_DIFF}")
		endif()
	endif()
endif()

configure_file(${SRC} ${DST} @ONLY)