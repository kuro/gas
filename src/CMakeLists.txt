
add_definitions(-Wall -Werror)
add_definitions(-pedantic -ansi)

set(sources gas.h gas.c)
add_library(gas SHARED ${sources})

install(TARGETS gas LIBRARY DESTINATION lib)
install(FILES gas.h DESTINATION include)
