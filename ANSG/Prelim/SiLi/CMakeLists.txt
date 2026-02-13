cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(GeSimulation)

find_package(Geant4 REQUIRED ui_all vis_all)
find_package(ROOT REQUIRED COMPONENTS RIO Core Tree)

include(${Geant4_USE_FILE})
include_directories(${ROOT_INCLUDE_DIRS})

file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

file(GLOB MACRO_FILES "*.mac")

file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

add_executable(sim sim.cc ${sources} ${headers})
target_link_libraries(sim ${Geant4_LIBRARIES})
target_link_libraries(sim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

add_custom_target(GeSimulation DEPENDS sim)
