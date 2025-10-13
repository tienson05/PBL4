# This is lib file -> use add_library
add_library(UILib STATIC
    NetworkViewer.hpp
    NetworkViewer.cpp
)

# Cho phép các module khác include file header trong UI/
target_include_directories(UILib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link với Qt Widgets, Core và CoreLib
target_link_libraries(UILib PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    CoreLib
)
