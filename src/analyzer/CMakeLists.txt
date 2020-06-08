add_executable(analyzer
  main.c
  )
target_link_libraries(analyzer
  PRIVATE
  counter_api
  )
add_library(analyzer_api STATIC "")
target_sources(analyzer_api
  PRIVATE
  analyzer.c
  )
target_link_libraries(analyzer_api
  PRIVATE
  common
  )

add_subdirectory(counter)
add_subdirectory(p)
add_subdirectory(q)
