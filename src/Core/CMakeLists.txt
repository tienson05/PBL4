# This is lib file -> use add_library
add_library(CoreLib STATIC
    NetworkScanner.cpp
    NetworkScanner.hpp
    PacketInfo.hpp
)

# Đường dẫn tới thư viện libpcap trong thư mục third_party
set(PCAP_PATH "${CMAKE_SOURCE_DIR}/third_party/libpcap")

# Thêm include và link directory (nếu có file libpcap.so ở đó)
include_directories(${PCAP_PATH}/include)
link_directories(${PCAP_PATH}/lib)

# Cho phép module khác (UI, App...) include CoreLib/
target_include_directories(CoreLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Liên kết libpcap
target_link_libraries(CoreLib PRIVATE
    ${PCAP_PATH}/lib/libpcap.so
    Qt${QT_VERSION_MAJOR}::Core
)
