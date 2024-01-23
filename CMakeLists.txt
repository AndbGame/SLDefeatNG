# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
project(SexLabDefeat VERSION 0.1 LANGUAGES CXX)
#set(VR_VERSION 2)
#set(AE_VERSION 1)

#option(BUILD_SKYRIMVR "Build for Skyrim VR" OFF)
#option(BUILD_SKYRIMAE "Build for Skyrim AE" OFF)

macro(set_from_environment VARIABLE)
	if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif()
endmacro()

include("CMakeUser.txt" OPTIONAL)

if(BUILD_SKYRIMAE)
	add_compile_definitions(SKYRIM_AE)
	add_compile_definitions(ENABLE_SKYRIM_AE)
#	set(CommonLibName "CommonLibSSE")
#	set_from_environment(SkyrimAEPath)
#	set(SkyrimPath ${SkyrimAEPath})
#	set(SkyrimVersion "Skyrim AE")
#	set(VERSION ${VERSION}.${AE_VERSION})
elseif(BUILD_SKYRIMVR)
#	add_compile_definitions(SKYRIMVR)
#	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
#	set(CommonLibName "CommonLibVR")
#	set_from_environment(SkyrimVRPath)
#	set(SkyrimPath ${SkyrimVRPath})
#	set(SkyrimVersion "Skyrim VR")
#	set(VERSION ${VERSION}.${VR_VERSION})
else()
#	set(CommonLibName "CommonLibSSE")
#	set_from_environment(Skyrim64Path)
#	set(SkyrimPath ${Skyrim64Path})
#	set(SkyrimVersion "Skyrim SSE")
endif()

# Otherwise, you can set OUTPUT_FOLDER to any place you'd like :)
# set(OUTPUT_FOLDER "C:/path/to/any/folder")

# Get the .cpp files from your src/ directory
# to be passed into add_commonlibsse_plugin
#
# This helper automatically finds all of your .cpp files for you
#
# Please note: the official recommendation is to list our every .cpp
# file individually after SOURCES
set(source_files ${source_files}
    src/DefeatUtils.cpp
    src/DefeatConfig.cpp
    src/DefeatWidget.cpp
    src/DefeatPapyrus.cpp
    src/DefeatHooks.cpp
    src/DefeatActor.cpp
    src/DefeatActorManager.cpp
    src/DefeatCombatManager.cpp
    src/DefeatManager.cpp
	src/PapyrusInterface/ExpiringFunctionCallQueue.cpp
	src/PapyrusInterface/ActorExtraDataCallQueue.cpp
)

# Setup your SKSE plugin as an SKSE plugin!
find_package(CommonLibSSE CONFIG REQUIRED)

add_custom_target(BuildVersion
  ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/BuildVersion.h.in
                   -D DST=${CMAKE_SOURCE_DIR}/BuildVersion.h
                   -P ${CMAKE_SOURCE_DIR}/GenerateBuildVersion.cmake
  )

add_commonlibsse_plugin(
    ${PROJECT_NAME} 
    SOURCES ${source_files} plugin.cpp
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE PCH.h) # <--- PCH.h is required!
target_include_directories(${PROJECT_NAME} PRIVATE include)
add_dependencies(${PROJECT_NAME} BuildVersion)

# When your SKSE .dll is compiled, this will automatically copy the .dll into your mods folder.
# Only works if you configure DEPLOY_ROOT above (or set the SKYRIM_MODS_FOLDER environment variable)
if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()

endif()
