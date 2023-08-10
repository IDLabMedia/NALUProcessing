# library
set( LIB_NAME NaluProcessingLib )

# get source files
file( GLOB BASE_SRC_FILES "*.cpp" )

# get include files
file( GLOB BASE_INC_FILES "*.h" )

# get all source files
set( SRC_FILES ${BASE_SRC_FILES})

# get all include files
set( INC_FILES ${BASE_INC_FILES})

# library
add_library( ${LIB_NAME} STATIC ${SRC_FILES} ${INC_FILES})

# set the folder where to place the projects
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER lib )