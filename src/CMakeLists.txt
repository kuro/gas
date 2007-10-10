
add_definitions(-Wall -Werror)

set(sources gas.h gas.c)
add_library(gas SHARED ${sources})

install(TARGETS gas LIBRARY DESTINATION lib)
install(FILES gas.h DESTINATION include)
