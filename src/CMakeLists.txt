add_subdirectory(analyzer)

add_library(common STATIC "")
target_sources(common
  PRIVATE
  process.c
  logger.c
  utils.c
  protocol.c
  list.c
  )
