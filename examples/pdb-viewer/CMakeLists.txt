find_package(Qt4 4.6 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
set(QT_USE_QTOPENGL TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(../../include)

set(SOURCES pdbviewerexample.cpp)
set(MOC_HEADERS pdbviewerexample.h)
set(UI_FORMS pdbviewerexample.ui)

qt4_wrap_ui(UI_SOURCES ${UI_FORMS})
qt4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

add_executable(pdb-viewer ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES})
target_link_libraries(pdb-viewer chemkit chemkit-graphics ${QT_LIBRARIES})
