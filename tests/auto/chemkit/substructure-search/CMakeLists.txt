qt4_wrap_cpp(MOC_SOURCES substructuresearchtest.h)
add_executable(substructuresearchtest substructuresearchtest.cpp ${MOC_SOURCES})
target_link_libraries(substructuresearchtest chemkit ${QT_LIBRARIES})
add_chemkit_test(substructure-search substructuresearchtest)
