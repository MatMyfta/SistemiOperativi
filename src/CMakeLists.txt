set(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR})
# Utility function to create a test target
function(UNITNOS_ADD_TEST _TARGET)
  add_executable(${_TARGET} "")
  target_sources(${_TARGET}
    PRIVATE
    ${CURRENT_DIR}/test_catch_main.cpp
    )
  target_link_libraries(${_TARGET}
    PRIVATE
    Catch2::Catch2
    )
  catch_discover_tests(${_TARGET}
    EXTRA_ARGS --use-colour yes)
  set_target_properties(${_TARGET}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )
endfunction()

add_subdirectory(containers)
add_subdirectory(analyzer)
add_subdirectory(main)

add_library(common STATIC "")
target_sources(common
  PRIVATE
  process.c
  logger.c
  utils.c
  protocol.c
  path_node.c
  )
target_link_libraries(common
  PUBLIC
  containers
  )
