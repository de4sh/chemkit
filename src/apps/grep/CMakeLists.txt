find_package(Qt4 4.6 COMPONENTS QtCore REQUIRED)
set(QT_DONT_USE_QTGUI TRUE)
include(${QT_USE_FILE})

include_directories(../../../include)

add_chemkit_executable(grep grep.cpp)
target_link_libraries(grep chemkit ${QT_LIBRARIES})
