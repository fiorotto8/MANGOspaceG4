cmake_minimum_required(VERSION 3.16)
project(B3a CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Geant4 ---
option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

# --- ROOT ---
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)

# --- Includes ---
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Sources/headers ---
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# --- Executable ---
add_executable(exampleB3a exampleB3a.cc  src/PhysicsList.cc src/G4LivermorePolarizedPhotoElectricGDModel.cc ${sources} ${headers})

# --- Link libs (Geant4 first or last is fine; keep both) ---
target_link_libraries(exampleB3a
  PRIVATE
    ${Geant4_LIBRARIES}
    ROOT::Core
    ROOT::RIO
    ROOT::Tree
)

# --- Runtime scripts copied next to the binary ---
set(EXAMPLEB3_SCRIPTS
  debug.mac
  exampleB3.in
  exampleB3.out
  init_vis.mac
  run1.mac
  run2.mac
  vis.mac
  myMac.mac
)
foreach(_script ${EXAMPLEB3_SCRIPTS})
  configure_file(${PROJECT_SOURCE_DIR}/${_script} ${PROJECT_BINARY_DIR}/${_script} COPYONLY)
endforeach()

# --- Convenience target & install ---
add_custom_target(B3a DEPENDS exampleB3a)
install(TARGETS exampleB3a DESTINATION bin)
