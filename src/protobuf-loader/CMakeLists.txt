option(NS3_PROTOBUF "Using Protobuf together with ns-3" OFF)

if(${NS3_PROTOBUF})
  include(FindProtobuf)
  find_package(Protobuf REQUIRED)
  include_directories(${PROTOBUF_INCLUDE_DIR})

  # compile protobuf definition files if they are changed
  add_custom_command(
	OUTPUT ${CMAKE_SOURCE_DIR}/src/protobuf-loader/model/topology.pb.h
	${CMAKE_SOURCE_DIR}/src/protobuf-loader/model/topology.pb.cc
	COMMAND ${Protobuf_PROTOC_EXECUTABLE} -I=protobuf --cpp_out=model --python_out=${CMAKE_SOURCE_DIR}/config/ protobuf/*.proto
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/protobuf-loader/
	DEPENDS protobuf/topology.proto
	COMMENT "Run protoc to generate protobuf files"
)

  build_lib(
	LIBNAME protobuf-loader
	SOURCE_FILES
	    helper/protobuf-topology-loader.cc
	    model/topology.pb.cc
	HEADER_FILES
        helper/protobuf-topology-loader.h
	    model/topology.pb.h
	LIBRARIES_TO_LINK
	    ${libcore}
	    ${libnetwork}
		${libpoint-to-point}
		${libapplications}
		${libdc-env}
		${libdcb}
	    ${PROTOBUF_LIBRARY}
	)
endif()
