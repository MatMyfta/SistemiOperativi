add_executable(counter
  main.c
  counter_self.c
  )
target_link_libraries(counter
  PRIVATE
  common
  counter_api
  p_api
  )

add_library(counter_api STATIC "")
target_sources(counter_api
  PRIVATE
  counter.c
  )
target_link_libraries(counter_api
  PRIVATE
  common
  )
