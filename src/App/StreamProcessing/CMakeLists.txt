# executable
set( EXE_NAME StreamProcessing )

# get source files
file( GLOB SRC_FILES "*.cpp" )

# get include files
file( GLOB INC_FILES "*.h" )

# add executable
add_executable( ${EXE_NAME} ${SRC_FILES} ${INC_FILES})

target_include_directories( ${EXE_NAME} PRIVATE ../../Lib)
target_link_libraries( ${EXE_NAME} NaluProcessingLib)

# set the folder where to place the projects
set_target_properties( ${EXE_NAME}  PROPERTIES FOLDER app LINKER_LANGUAGE CXX )
