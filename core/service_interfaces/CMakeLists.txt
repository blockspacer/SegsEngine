set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreInterface.h
)
add_library(core_service_interfaces INTERFACE )
#target_sources(core_service_interfaces INTERFACE ${SRC})
target_include_directories(core_service_interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
