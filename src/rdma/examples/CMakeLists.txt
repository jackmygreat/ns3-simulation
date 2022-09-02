build_lib_example(
  NAME rdma-example
  SOURCE_FILES rdma-example.cc
  LIBRARIES_TO_LINK ${libcore}
                    ${librdma}
)
