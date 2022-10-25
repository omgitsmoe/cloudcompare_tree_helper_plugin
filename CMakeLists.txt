cmake_minimum_required( VERSION 3.0 )

# CloudCompare example for standard plugins

# REPLACE ALL 'ExamplePlugin' OCCURENCES BY YOUR PLUGIN NAME
# AND ADAPT THE CODE BELOW TO YOUR OWN NEEDS!

# Add an option to CMake to control whether we build this plugin or not
option( PLUGIN_STANDARD_QTREEISO "Check to install tree isolation helper plugin" OFF )

if ( PLUGIN_STANDARD_QTREEISO )
	# Name the plugin
    project( qTreeIso )
    
    # load any subdirectories (see qAdditionalIO for an example)
    # add_subdirectory( LIB1 )
    
    # set dependencies to necessary libraries (see qPCV for an example)
    # target_link_libraries( ${PROJECT_NAME} LIB1 )
    # include_directories( ${LIB1_INCLUDE_DIR} )
    
    #we need picking hub classes
	list( APPEND CC_PLUGIN_CUSTOM_HEADER_LIST ${CloudCompare_SOURCE_DIR}/../common/ccPickingListener.h )
	list( APPEND CC_PLUGIN_CUSTOM_HEADER_LIST ${CloudCompare_SOURCE_DIR}/../common/ccPickingHub.h )
	list( APPEND CC_PLUGIN_CUSTOM_SOURCE_LIST ${CloudCompare_SOURCE_DIR}/../common/ccPickingHub.cpp )
    
    include( ../../CMakePluginTpl.cmake )
endif()
