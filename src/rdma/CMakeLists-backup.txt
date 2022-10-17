build_lib(
  LIBNAME rdma
  SOURCE_FILES
      model/rdma.cc
	  model/rdma-tx-queue-pair.cc
	  model/rdma-rx-queue-pair.cc
	  helper/rdma-helper.cc
  HEADER_FILES
      model/rdma.h
	  model/rdma-tx-queue-pair.h
	  model/rdma-rx-queue-pair.h
	  helper/rdma-helper.h
  LIBRARIES_TO_LINK
      ${libcore}
	  ${libinternet}
)
