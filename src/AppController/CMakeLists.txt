# This is executable file -> use add_execuable
add_executable(App
    main.cpp
)

# target_link_libraries(App
#     PRIVATE Qt::Core
#     PRIVATE Qt::Widgets
# )

target_link_libraries(App PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    CoreLib
    UILib
)

# add_custom_command(TARGET App POST_BUILD
#     COMMAND setcap cap_net_raw,cap_net_admin=eip $<TARGET_FILE:App>
#     COMMENT "Setting network capture permissions on App"
# )
