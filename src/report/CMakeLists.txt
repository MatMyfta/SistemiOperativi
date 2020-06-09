add_executable(report
  main.c
  report.c
  )
target_link_libraries(report
  PRIVATE
  common
  )
