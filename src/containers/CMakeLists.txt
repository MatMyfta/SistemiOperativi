add_library(containers STATIC "")
target_sources(containers
  PRIVATE
  list.c
  tree.c
  )

unitnos_add_test(tree-test)
target_sources(tree-test
  PRIVATE
  tree.test.cpp
  )
target_link_libraries(tree-test
  PRIVATE
  containers
  )
