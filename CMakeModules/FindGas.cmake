find_path(
    GAS_INCLUDE_DIR
    NAMES gas/tree.h
    PATHS
    ${GAS_DIR}/include
    ${Gas_SOURCE_DIR}/src
    ${GAS_SOURCE_DIR}/src  # user specified
    )

find_library(
    GAS_LIBRARY
    NAMES gas
    PATHS
    /usr/lib
    /usr/local/lib
    ${GAS_DIR}/lib
    ${Gas_BINARY_DIR}/src/gas
    ${GAS_BINARY_DIR}/src/gas  # user specified
    )

if (GAS_LIBRARY AND GAS_INCLUDE_DIR)
    set(GAS_FOUND TRUE)
else (GAS_LIBRARY AND GAS_INCLUDE_DIR)
    set(GAS_FOUND FALSE)
endif (GAS_LIBRARY AND GAS_INCLUDE_DIR)

mark_as_advanced(GAS_INCLUDE_DIR GAS_LIBRARY)
