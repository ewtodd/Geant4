cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(GeSimulation)

# Find Geant4 package
find_package(Geant4 REQUIRED ui_all vis_all)
# Find ROOT package
find_package(ROOT REQUIRED COMPONENTS Hist RIO Net)

# Include ROOT directories
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIR})

# Include Geant4 configurations
include(${Geant4_USE_FILE})

# Source files and header files
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

# Crystal Maps and Macros
set(GeSimulation_maps
    CrystalMaps/Ge/config.txt
    CrystalMaps/Ge/FT.ssv
    CrystalMaps/Ge/FTVec.ssv
    CrystalMaps/Ge/L.ssv
    CrystalMaps/Ge/LVec.ssv
    CrystalMaps/Ge/ST.ssv
    CrystalMaps/Ge/STVec.ssv)
file(GLOB MACRO_FILES "*.mac")

# Copy maps and macros
foreach(_map ${GeSimulation_maps})
  file(COPY ${_map} DESTINATION ${PROJECT_BINARY_DIR}/Ge/)
endforeach()
file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

# Add executable
add_executable(sim ${sources} ${headers})
target_link_libraries(sim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

# Custom target
add_custom_target(GeSimulation DEPENDS sim)
