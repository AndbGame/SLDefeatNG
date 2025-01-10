# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
project(SexLabDefeat VERSION 0.2.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include("CMakeUser.txt" OPTIONAL)

# #######################################################################################################################
# # Add CMake features
# #######################################################################################################################
include(XSEPlugin)

# #######################################################################################################################
# # Find dependencies
# #######################################################################################################################
find_package(magic_enum CONFIG REQUIRED)
find_package(xbyak CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS 
	stacktrace_basic
	property_tree
	algorithm
)
find_library(Detours_LIBRARIES NAMES detours.lib)
find_package(imgui CONFIG REQUIRED)

find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")

target_include_directories(
	${PROJECT_NAME} 
	PRIVATE 
	include
	magic_enum::magic_enum
	xbyak::xbyak
	nlohmann_json::nlohmann_json
	Microsoft::DirectXTex
	Boost::property_tree
	Boost::algorithm
	imgui::imgui
	${CLIB_UTIL_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
		Boost::stacktrace_basic
        ${Detours_LIBRARIES}
)

#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
#target_precompile_headers(${PROJECT_NAME} PRIVATE PCH.h) # <--- PCH.h is required!


# https://gitlab.kitware.com/cmake/cmake/-/issues/24922#note_1371990
if(MSVC_VERSION GREATER_EQUAL 1936 AND MSVC_IDE) # 17.6+
	# When using /std:c++latest, "Build ISO C++23 Standard Library Modules" defaults to "Yes".
	# Default to "No" instead.
	#
	# As of CMake 3.26.4, there isn't a way to control this property
	# (https://gitlab.kitware.com/cmake/cmake/-/issues/24922),
	# We'll use the MSBuild project system instead
	# (https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure)
	file(CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/Directory.Build.props" CONTENT [==[
<Project>
  <ItemDefinitionGroup>
    <ClCompile>
      <BuildStlModules>false</BuildStlModules>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
]==] @ONLY)
endif()

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
    #if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    #endif()

endif()
