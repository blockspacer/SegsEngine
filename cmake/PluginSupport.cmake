set(pluginBuildTypes Disabled Static Shared)

macro(set_plugin_options )

    set(oneValueArgs NAME CLASSPROP TYPE SHARED DISABLED)
    set(multiValueArgs INCLUDES DEFINES SOURCES LIBS)
    cmake_parse_arguments(plugin_options "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set(name ${plugin_options_NAME})
    set(classprop ${plugin_options_CLASSPROP})
    set(includes ${plugin_options_INCLUDES})
    set(sources ${plugin_options_SOURCES})
    set(defines ${plugin_options_DEFINES})
    set(libs ${plugin_options_LIBS})
    set(when_shared_libs ${plugin_options_WHEN_SHARED_LIBS})
    set(group_folder ${plugin_options_TYPE})
    set(default_shared ${plugin_options_SHARED})
    set(default_disabled ${plugin_options_DISABLED})
    set(default_mode Static)
    if(default_shared)
        set(default_mode Shared)
    endif()
    if(default_disabled)
        set(default_mode Disabled)
    endif()
    foreach(tgt ${global_targets})
        set(tgt_name ${tgt}_plugin_${name})
        set(${tgt_name}_mode ${default_mode} CACHE STRING "How to integrate the given plugin into the build")
        set_property(CACHE ${tgt_name}_mode PROPERTY STRINGS ${pluginBuildTypes})
        if(${${tgt_name}_mode} STREQUAL "Static")
            #message("STATIC LINKING OF ${tgt_name}")
            add_library(${tgt_name} STATIC)
            target_compile_definitions(${tgt_name} PRIVATE QT_STATICPLUGIN)
            # this is needed for plugin_registry.cpp includes
            target_link_libraries(${tgt_name} PUBLIC ${tgt}_core)
            set_common_target_properties(${tgt_name})
        elseif(${${tgt_name}_mode} STREQUAL "Shared")
            #message("DYNAMIC LINKING OF ${tgt_name}")
            add_library(${tgt_name} SHARED)
            set_target_properties(${tgt_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/plugins)
            set_target_properties(${tgt_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/plugins)
            target_link_libraries(${tgt_name} PRIVATE ${tgt}_engine)
            install(TARGETS ${tgt_name} EXPORT SegsEngine
                LIBRARY DESTINATION bin/plugins
                RUNTIME DESTINATION bin/plugins
            )
        else()
            continue()
        endif()
        target_link_libraries(${tgt_name} PRIVATE core_service_interfaces)
        target_link_libraries(${tgt_name} PRIVATE Qt5::Core EASTL_Import)
        set_target_properties(${tgt_name} PROPERTIES AUTOMOC TRUE)
        if(${${tgt_name}_mode} STREQUAL "Shared")
            if(when_shared_libs)
                target_link_libraries(${tgt_name} PRIVATE ${when_shared_libs})
            elseif(libs)
                target_link_libraries(${tgt_name} PRIVATE ${libs})
            endif()
        else()
        if(libs)
            target_link_libraries(${tgt_name} PRIVATE ${libs})

            endif()
        endif()
        set_target_properties(${tgt_name} PROPERTIES PLUGIN_CLASS ${classprop})
        target_sources(${tgt_name} PRIVATE ${sources})
        if(includes)
            target_include_directories(${tgt_name} PRIVATE ${includes})
        endif()
        if(defines)
            target_compile_definitions(${tgt_name} PUBLIC ${defines})
        endif()
        set_target_properties (${tgt_name} PROPERTIES
            FOLDER plugins/${group_folder}
        )
    endforeach()
endmacro()

function(collect_plugins for_target tgt_var)
    set(collected_static)
    set(collected_disabled)
    set(collected_shared)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})

        unset(MATCHED)
        string(REGEX MATCH "${for_target}_plugin_([A-Za-z0-9_]+)_mode" MATCHED ${_variableName})
        if (NOT MATCHED)
            continue()
        endif()
        set(target_name ${CMAKE_MATCH_1})

        if(${${_variableName}} STREQUAL "Disabled")
            list(APPEND collected_disabled ${target_name})
        elseif(${${_variableName}} STREQUAL "Static")
            list(APPEND collected_static ${target_name})
        elseif(${${_variableName}} STREQUAL "Shared")
            list(APPEND collected_shared ${target_name})
        else()
            message(SEND_ERROR "Unkown plugin mode for ${target_name} ${${_variableName}}")
        endif()
    endforeach()
    set(${tgt_var}_disabled ${collected_disabled} PARENT_SCOPE)
    set(${tgt_var}_static ${collected_static} PARENT_SCOPE)
    set(${tgt_var}_shared ${collected_shared} PARENT_SCOPE)
endfunction()

function(generate_static_plugins_list target global_tgt plugins)
    set(parts)
    #message(" =========================== ${plugins}")
    foreach(plug ${plugins})
        set(tgt_name ${global_tgt}_plugin_${plug})
        get_target_property(plug_name ${tgt_name} PLUGIN_CLASS)
        list(APPEND parts "Q_IMPORT_PLUGIN(${plug_name})")
    endforeach()
    string(JOIN "\n" importers ${parts})
    # file generated in binary directory of calling target
    configure_file(${PROJECT_SOURCE_DIR}/plugins/static_plugin_registry.cpp.in ${global_tgt}_static_plugin_registry.cpp)
endfunction()

function(add_plugins_to_target target global_tgt)
    SET(plugins)

    collect_plugins(${global_tgt} plugins)
    #we create a new target that will contain the selected plugins
    generate_static_plugins_list(${target} ${global_tgt} "${plugins_static}")
    target_sources(${target} PRIVATE ${PROJECT_SOURCE_DIR}/plugins/plugin_registry.cpp)
    target_sources(${target} PRIVATE ${global_tgt}_static_plugin_registry.cpp)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    foreach(plug ${plugins_static})
        set(tgt_name ${global_tgt}_plugin_${plug})
        target_link_libraries(${target} PUBLIC ${tgt_name})
    endforeach()
    target_link_libraries(${target} PRIVATE Qt5::Core EASTL_Import)
endfunction()
