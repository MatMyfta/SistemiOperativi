add_executable(q
  main.c
  q_self.c
  )
target_link_libraries(q
  PRIVATE
  common
  q_api
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
