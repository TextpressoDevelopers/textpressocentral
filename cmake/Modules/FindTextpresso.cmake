# find Textpresso core library

FIND_PATH( Textpresso_INCLUDE_DIR NAMES CASManager.h IndexManager.h PATHS ENV PATH PATH_SUFFIXES
        include textpresso)

FIND_LIBRARY( Textpresso_LIBRARY NAMES textpresso PATHS PATH PATH_SUFFIXES lib lib-release lib_release )

IF( Textpresso_LIBRARY )
    SET( Textpresso_FOUND TRUE )
    SET( Textpresso_LIBRARIES Textpresso_LIBRARY )
ENDIF( Textpresso_LIBRARY)

IF( Textpresso_FOUND )
    IF (NOT Textpresso_FIND_QUIETLY)
        MESSAGE(STATUS "Found the Textpresso libraries at ${Textpresso_LIBRARY}")
        MESSAGE(STATUS "Found the Textpresso headers at ${Textpresso_INCLUDE_DIR}")
    ENDIF (NOT Textpresso_FIND_QUIETLY)
ELSE( Textpresso_FOUND )
    IF(Textpresso_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could NOT find Textpresso")
    ENDIF(Textpresso_FIND_REQUIRED)
ENDIF(Textpresso_FOUND)