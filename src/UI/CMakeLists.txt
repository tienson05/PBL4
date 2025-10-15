# --- Bật tự động xử lý Qt ---
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# --- Thư viện UI ---
add_library(UILib STATIC
    NetworkViewer.hpp
    NetworkViewer.cpp
    WelcomePage.hpp
    WelcomePage.cpp
    MainWindow.cpp
    MainWindow.hpp
    HeaderWidget.hpp
    HeaderWidget.cpp
    CapturePage.hpp
    CapturePage.cpp
)

# Cho phép các module khác include file header trong UI/
target_include_directories(UILib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link với Qt Widgets, Core và CoreLib
target_link_libraries(UILib PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    CoreLib
)
