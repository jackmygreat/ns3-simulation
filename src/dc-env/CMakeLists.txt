build_lib(
  LIBNAME dc-env
  SOURCE_FILES
	  model/dc-topology.cc
	  model/dc-switch.cc
	  model/dc-host.cc
  HEADER_FILES
	  model/dc-topology.h
      model/dc-switch.h
	  model/dc-host.h
  LIBRARIES_TO_LINK
      ${libcore}
      ${libnetwork}
	  ${libinternet}
)
