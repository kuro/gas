find_path(
    GAS_INCLUDE_DIR
    gas/tree.h
    ${GAS_DIR}/include
    ${GAS_SOURCE_DIR}/src
    )

find_library(
    GAS_LIBRARY
    NAMES gas
    PATHS /usr/lib /usr/local/lib ${GAS_DIR}/lib ${GAS_BINARY_DIR}/src/gas
    )

if (GAS_LIBRARY AND GAS_INCLUDE_DIR)
    set(GAS_FOUND TRUE)
else (GAS_LIBRARY AND GAS_INCLUDE_DIR)
    set(GAS_FOUND FALSE)
endif (GAS_LIBRARY AND GAS_INCLUDE_DIR)

mark_as_advanced(GAS_INCLUDE_DIR GAS_LIBRARY)
