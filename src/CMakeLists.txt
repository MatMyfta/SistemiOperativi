add_subdirectory(analyzer)

add_library(common)
target_sources(common
  PRIVATE
  process.c
  logger.c
  utils.c
  protocol.c
  )
