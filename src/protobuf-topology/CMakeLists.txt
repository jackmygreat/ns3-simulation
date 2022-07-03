option(NS3_PROTOBUF "Using Protobuf together with ns-3" OFF)

if(${NS3_PROTOBUF})
  include(FindProtobuf)
  find_package(Protobuf REQUIRED)
  include_directories(${PROTOBUF_INCLUDE_DIR})

  add_custom_target(
	compile_protobuf
	COMMAND ${Protobuf_PROTOC_EXECUTABLE} -I=protobuf --cpp_out=model --python_out=${CMAKE_SOURCE_DIR}/config/ protobuf/topology.proto
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/protobuf-topology/
	)

  build_lib(
	LIBNAME protobuf-topology
	SOURCE_FILES
	    helper/protobuf-topology-loader.cc
		model/dc-topology.cc
	    model/topology.pb.cc
	HEADER_FILES
        helper/protobuf-topology-loader.h
		model/dc-topology.h
	    model/topology.pb.h
	LIBRARIES_TO_LINK
	    ${libnetwork}
	    ${PROTOBUF_LIBRARY}
	DEPENDENCIES
	    compile_protobuf # compile protobuf before building the module
	)
endif()
