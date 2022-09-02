build_lib(
  LIBNAME dcb
  SOURCE_FILES
      helper/dcb-stack-helper.cc
  HEADER_FILES
      helper/dcb-stack-helper.h
  LIBRARIES_TO_LINK
      ${libcore}
	  ${libnetwork}
	  ${libinternet}
)
