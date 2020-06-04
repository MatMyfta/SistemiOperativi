add_subdirectory(analyzer)
add_subdirectory(main)

add_library(common STATIC "")
target_sources(common
  PRIVATE
  process.c
  logger.c
  utils.c
  protocol.c
  list.c
  tree.c
  )
