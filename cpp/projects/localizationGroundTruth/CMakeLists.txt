cmake_minimum_required (VERSION 2.8)
project(StereoTests)

# Collect all sources
file(GLOB_RECURSE LOCALIZATIONGT_SOURCE_FILES "src/*.cpp" "src/*.h" "src/*.inl")

# Compiler specific options
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()

# Loading 3rd party libraries.
find_package(OpenCV 3 REQUIRED core imgproc ml highgui features2d xfeatures2d calib3d objdetect)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
LINK_DIRECTORIES(${OpenCV_LIB_DIR})


find_package(PCL 1.2 REQUIRED common io kdtree filters visualization segmentation)
if(${PCL_FOUND} AND ${VTK_FOUND} AND ${Boost_FOUND} AND ${FLANN_FOUND})
	add_definitions(-DENABLE_PCL)
	include_directories(${PCL_INCLUDE_DIRS})
	link_directories(${PCL_LIBRARY_DIRS})
	add_definitions(${PCL_DEFINITIONS})
endif()

# Add Executable
add_executable(localizationGT ${LOCALIZATIONGT_SOURCE_FILES})

target_include_directories(localizationGT PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(localizationGT PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(localizationGT PUBLIC ${PROJECT_SOURCE_DIR}/../../lib/StereoLib/src)
target_link_libraries(localizationGT LINK_PUBLIC StereoLib)
target_link_libraries(localizationGT LINK_PUBLIC ${OpenCV_LIBS})
target_link_libraries(localizationGT LINK_PUBLIC ${PCL_LIBRARIES})

