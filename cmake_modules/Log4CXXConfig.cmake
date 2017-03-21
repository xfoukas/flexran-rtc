# LOG4CXX_FOUND - system has liblog4cxx
# LOG4CXX_INCLUDE_DIR - the liblog4cxx include directory
# LOG4CXX_LIBRARIES - liblog4cxx library

FIND_PATH(Log4CXX_INCLUDE_DIR logger.h PATHS /include/log4cxx /usr/include/log4cxx /usr/local/include/log4cxx )
FIND_LIBRARY(Log4CXX_LIBRARIES NAMES log4cxx log4cxxd PATHS /lib /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu )

IF(Log4CXX_INCLUDE_DIR AND Log4CXX_LIBRARIES)
  SET(Log4CXX_FOUND 1)
  #remove last /log4cxx string
  STRING(REGEX REPLACE "/log4cxx" " " Log4CXX_INCLUDE_DIR_SUP_LEVEL ${Log4CXX_INCLUDE_DIR})
  SET (Log4CXX_INCLUDE_DIR ${Log4CXX_INCLUDE_DIR_SUP_LEVEL} ${Log4CXX_INCLUDE_DIR} )
  if(NOT Log4CXX_FIND_QUIETLY)
    message(STATUS "Found log4cxx: ${Log4CXX_LIBRARIES}" ", " ${Log4CXX_INCLUDE_DIR})
  endif(NOT Log4CXX_FIND_QUIETLY)
ELSE(Log4CXX_INCLUDE_DIR AND Log4CXX_LIBRARIES)
  SET(Log4CXX_FOUND 0 CACHE BOOL "Not found log4cxx library")
  message(STATUS "NOT Found log4cxx")
ENDIF(Log4CXX_INCLUDE_DIR AND Log4CXX_LIBRARIES)

MARK_AS_ADVANCED(Log4CXX_INCLUDE_DIR Log4CXX_LIBRARIES)
