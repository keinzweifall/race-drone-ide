project(race-drone-ide)

cmake_minimum_required(VERSION 3.2)
#cmake_minimum_required(VERSION 2.8)

# use boost 1.60
#set(Boost_NO_SYSTEM_PATHS ON)
#set(BOOST_ROOT "/home/hans/Development/cpp/boost_1_60_0/install")
#set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
#set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

add_definitions(-DBOOST_LOG_DYN_LINK)

set(RAPIDJSON_INCLUDEDIR /home/hans/Development/cpp/rapidjson/include)

# use parrot PARROT_SDK
set(PARROT_SDK_ROOT /home/hans/Development/cpp/parrot-drone-sdk/ARSDKBuildUtils/Targets/Unix/Install)
set(PARROT_SDK_INCLUDEDIR ${PARROT_SDK_ROOT}/include)
set(PARROT_SDK_LIBDIR ${PARROT_SDK_ROOT}/lib)
find_library(PARROT_LIBARSAL name arsal PATHS ${PARROT_SDK_ROOT}/lib)
find_library(PARROT_LIBARCOMMAND name arcommands PATHS ${PARROT_SDK_ROOT}/lib)
find_library(PARROT_LIBARNETWORK name arnetwork PATHS ${PARROT_SDK_ROOT}/lib)
find_library(PARROT_LIBARNETWORKAL name arnetworkal PATHS ${PARROT_SDK_ROOT}/lib)
find_library(PARROT_LIBARDISCOVERY name ardiscovery PATHS ${PARROT_SDK_ROOT}/lib)
find_library(PARROT_LIBARSTREAM name arstream PATHS ${PARROT_SDK_ROOT}/lib)

find_package(Boost COMPONENTS thread system filesystem unit_test_framework log log_setup REQUIRED)
find_package(Threads REQUIRED)
find_package(Qt4 REQUIRED)

qt4_wrap_ui(projectUI src/ui/qdronedesktop.ui src/ui/qanimationcontrol.ui src/ui/qdrivecontrol.ui src/ui/qdronedashboard.ui src/ui/qvideodisplay.ui)
qt4_wrap_cpp(projectMoc src/ui/qimageprovider.h src/ui/qdronedesktop.h src/ui/qanimationcontrol.h src/ui/qdrivecontrol.h src/ui/qdronedashboard.h src/ui/qvideodisplay.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} ${Boost_INCLUDE_DIRS} ${PARROT_SDK_INCLUDEDIR} ${RAPIDJSON_INCLUDEDIR} ${CMAKE_SOURCE_DIR}/src)

set(race-drone-ide_SRCS src/model/videoframefeed.cpp src/ui/qimageprovider.cpp src/ui/qvideodisplay.cpp src/ui/qvideodisplay.cpp src/controller/devicecontroller.cpp src/ui/qdronedesktop.ui src/ui/qdronedesktop.cpp src/ui/qanimationcontrol.ui src/ui/qanimationcontrol.cpp src/ui/qdronedashboard.ui src/ui/qdronedashboard.cpp src/ui/qdrivecontrol.ui src/ui/qdrivecontrol.cpp src/main.cpp)

add_executable(race-drone-ide ${race-drone-ide_SRCS} ${projectUI} ${projectMoc})
set_property(TARGET race-drone-ide PROPERTY CXX_STANDARD 11)
set_property(TARGET race-drone-ide PROPERTY CXX_STANDARD_REQUIRED ON)

set(Boost_LIBS_NEEDED ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
message(${Boost_LIBS_NEEDED})

set(PARROT_SDK_LIBS ${PARROT_LIBARSAL} ${PARROT_LIBARCOMMAND} ${PARROT_LIBARDISCOVERY} ${PARROT_LIBARNETWORK} ${PARROT_LIBARNETWORKAL} ${PARROT_LIBARSTREAM})
message(${PARROT_SDK_LIBS})

target_link_libraries(race-drone-ide  ${CMAKE_THREAD_LIBS_INIT} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${Boost_LIBS_NEEDED} ${PARROT_SDK_LIBS})

install(TARGETS race-drone-ide RUNTIME DESTINATION bin)
