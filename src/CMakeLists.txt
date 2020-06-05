add_subdirectory(containers)
add_subdirectory(analyzer)
add_subdirectory(main)

add_library(common STATIC "")
target_sources(common
  PRIVATE
  process.c
  logger.c
  utils.c
  protocol.c
  path_node.c
  )
target_link_libraries(common
  PUBLIC
  containers
  )
