
set(FIND_HEXANOISE_LIB_SUFFIX "")
if(HEXANOISE_STATIC_LIBRARY)
    set(FIND_HEXANOISE_LIB_SUFFIX "-s")
endif()

find_path(HEXANOISE_INCLUDE_DIRS hexanoise/generator_context.hpp
    PATH_SUFFIXES include
    PATHS
        $ENV{BUILD_ROOT}
        /usr/local
        /usr
        ~/Library
        /Library
        /opt/local
        /opt
        )

find_library(HEXANOISE_LIBRARIES
    NAMES libhexanoise${FIND_HEXANOISE_LIB_SUFFIX} hexanoise${FIND_HEXANOISE_LIB_SUFFIX}
    PATH_SUFFIXES lib64 lib
    PATHS
        $ENV{BUILD_ROOT}
        /usr/local
        /usr
        ~/Library
        /Library
        /opt/local
        /opt
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HEXANOISE DEFAULT_MSG
                                  HEXANOISE_LIBRARIES HEXANOISE_INCLUDE_DIRS)

