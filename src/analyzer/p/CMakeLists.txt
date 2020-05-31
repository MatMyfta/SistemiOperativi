add_executable(p
  main.c
  p_self.c
  )
target_link_libraries(p
  PRIVATE
  common
  p_api
  q_api
  )

add_library(p_api STATIC "")
target_sources(p_api
  PRIVATE
  p.c
  )
target_link_libraries(p_api
  PRIVATE
  common
  )
