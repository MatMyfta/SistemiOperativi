add_library(containers STATIC "")
target_sources(containers
  PRIVATE
  base.c
  list.c
  tree.c
  set.c
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
