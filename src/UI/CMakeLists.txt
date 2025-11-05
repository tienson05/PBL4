# --- Bật tự động xử lý Qt ---
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# --- Thư viện UI ---
# Tốt hơn là nên liệt kê file theo đúng đường dẫn trong dự án
add_library(UILib STATIC
    CapturePage.hpp
    CapturePage.cpp

    HeaderWidget.hpp
    HeaderWidget.cpp

   MainWindow.hpp
    MainWindow.cpp

    NetworkViewer.hpp
    NetworkViewer.cpp

    WelcomePage.hpp
    WelcomePage.cpp
    StatisticsDialog.hpp StatisticsDialog.cpp
    StatisticsManager.hpp StatisticsManager.cpp
)

# Cho phép các module khác include file header trong UI/
target_include_directories(UILib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link với Qt Widgets, Core, Network và CoreLib
target_link_libraries(UILib PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets

    # THÊM DÒNG NÀY ĐỂ SỬA LỖI
    Qt${QT_VERSION_MAJOR}::Network

    CoreLib
)
