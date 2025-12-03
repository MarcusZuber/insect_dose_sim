cmake_minimum_required(VERSION 3.16)
project(insect_dose_sim)

set(CMAKE_CXX_STANDARD 17)


# If the user hasn't set Geant4_DIR in their environment, provide a sensible default
if (NOT DEFINED Geant4_DIR OR NOT Geant4_DIR)
    set(Geant4_DIR "~/src/geant4_install/" CACHE PATH "Path to Geant4 installation directory")
endif ()

list(APPEND CMAKE_PREFIX_PATH ${Geant4_DIR}/lib/cmake/Geant4)

# Find Geant4 package
find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

add_executable(insect_dose_sim
        main.cpp
        src/DetectorConstruction.cpp
        src/DetectorMessenger.cpp
        src/PhysicsList.cpp
        src/PrimaryGeneratorAction.cpp
        src/PrimaryGeneratorMessenger.cpp
        src/ActionInitialization.cpp
        src/RunAction.cpp
        src/RunMessenger.cpp
        src/EventAction.cpp
        src/SteppingAction.cpp
        include/parameters.h
        include/DetectorConstruction.h
        include/DetectorMessenger.h
        include/PhysicsList.h
        include/PrimaryGeneratorAction.h
        include/PrimaryGeneratorMessenger.h
        include/ActionInitialization.h
        include/RunAction.h
        include/EventAction.h
        include/SteppingAction.h
        include/RunMessenger.h
)

# Include directories
target_include_directories(insect_dose_sim PRIVATE include)

# Link Geant4 libraries
target_link_libraries(insect_dose_sim ${Geant4_LIBRARIES})

# Ensure the built executable can find Geant4 shared libraries at runtime without sourcing geant4.sh
# Compute a likely lib directory from Geant4_DIR. Geant4_DIR usually points to <prefix>/lib/cmake/Geant4
get_filename_component(_g4_cmake_dir ${Geant4_DIR}/lib/cmake/Geant4 REALPATH)
get_filename_component(_g4_parent "${_g4_cmake_dir}" DIRECTORY)
get_filename_component(_g4_root "${_g4_parent}" DIRECTORY)
set(GEANT4_LIB_DIR "${_g4_root}/lib")

# If the path doesn't exist, try a fallback using CMAKE_PREFIX_PATH first entry
if (NOT EXISTS "${GEANT4_LIB_DIR}")
    list(GET CMAKE_PREFIX_PATH 0 _first_prefix)
    set(GEANT4_LIB_DIR "${_first_prefix}/../lib")
endif ()

# Make sure RPATHs are set so the binary can find Geant4 libs without extra env setup
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set_target_properties(insect_dose_sim PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "${GEANT4_LIB_DIR}"
        BUILD_RPATH "${GEANT4_LIB_DIR}"
)

# Copy asset directories at build time, but only when source files changed
set(ASSET_DIRS macros meshes spectra)
set(ASSET_STAMPS)

foreach (dir IN LISTS ASSET_DIRS)
    # collect all files in the source asset directory and re-evaluate the glob when files change
    file(GLOB_RECURSE ASSET_FILES_${dir} CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${dir}/*")

    # stamp file path in build dir
    set(STAMP_FILE "${CMAKE_BINARY_DIR}/${dir}.stamp")
    list(APPEND ASSET_STAMPS ${STAMP_FILE})

    add_custom_command(
            OUTPUT ${STAMP_FILE}
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/${dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/${dir}" "${CMAKE_BINARY_DIR}/${dir}"
            COMMAND ${CMAKE_COMMAND} -E touch ${STAMP_FILE}
            DEPENDS ${ASSET_FILES_${dir}}
            COMMENT "Updating asset directory: ${dir} -> ${CMAKE_BINARY_DIR}/${dir}"
            VERBATIM
    )
endforeach ()

# Aggregate target that depends on all stamp files; will only run copy commands if stamps are out-of-date
add_custom_target(copy_assets DEPENDS ${ASSET_STAMPS} COMMENT "Ensure asset directories are up-to-date")

# Make the executable depend on the copy step so assets exist in the build dir before running
add_dependencies(insect_dose_sim copy_assets)
