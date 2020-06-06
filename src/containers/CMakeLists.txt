add_library(containers STATIC "")
target_sources(containers
  PRIVATE
  list.c
  tree.c
  dictionary.c
  )

unitnos_add_test(containers-test)
target_sources(containers-test
  PRIVATE
  tree.test.cpp
  dictionary.test.cpp
  )
target_link_libraries(containers-test
  PRIVATE
  containers
  )
