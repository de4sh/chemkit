find_package(Qt4 4.6 COMPONENTS QtCore REQUIRED)
set(QT_DONT_USE_QTGUI TRUE)
include(${QT_USE_FILE})

set(SOURCES
  rotatablebondsdescriptor.cpp
  rotatablebondsplugin.cpp
)

set(MOC_HEADERS
  rotatablebondsplugin.h
)

qt4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

add_chemkit_plugin(rotatablebonds ${SOURCES} ${MOC_SOURCES})
target_link_libraries(rotatablebonds chemkit ${QT_LIBRARIES})
