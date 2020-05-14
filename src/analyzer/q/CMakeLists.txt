add_executable(q)
target_sources(q
  PRIVATE
  main.c
  )

add_library(q_api)
target_sources(q_api
  PRIVATE
  q.c
  )
target_link_libraries(q_api
  PRIVATE
  common
  )
