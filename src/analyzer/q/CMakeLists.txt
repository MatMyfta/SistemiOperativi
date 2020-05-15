add_executable(q
  main.c
  )

add_library(q_api STATIC "")
target_sources(q_api
  PRIVATE
  q.c
  )
target_link_libraries(q_api
  PRIVATE
  common
  )
