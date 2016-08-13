# Install example configs
file(GLOB EXAMPLE_CONFIGS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.example")
install(FILES ${EXAMPLE_CONFIGS} DESTINATION ${CONF_PATH}/examples)
install(DIRECTORY aliases modules DESTINATION ${CONF_PATH}/examples)
