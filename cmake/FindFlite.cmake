# We can't create the same interface imported target multiple times, CMake will complain if we do
# that. This can happen if the find_package call is done in multiple different subdirectories.
if(TARGET Flite::Flite)
    set(Flite_FOUND 1)
    return()
endif()

find_path(FLITE_INCLUDE_DIR
    NAMES
        flite/flite.h
)
find_library(FLITE_LIBRARY
    NAMES
        flite
)

if(NOT FLITE_INCLUDE_DIR OR NOT FLITE_LIBRARY)
    set(Flite_FOUND 0)
    return()
endif()

include(CMakePushCheckState)
include(CheckCXXSourceCompiles)

# Flite can be built with ALSA support,
# in which case we need to link ALSA as well
find_package(ALSA QUIET)

cmake_push_check_state(RESET)

set(CMAKE_REQUIRED_INCLUDES "${FLITE_INCLUDE_DIR}")
set(CMAKE_REQUIRED_LIBRARIES "${FLITE_LIBRARY}")

if(ALSA_FOUND)
list(APPEND CMAKE_REQUIRED_LIBRARIES "${ALSA_LIBRARIES}")
endif()

check_cxx_source_compiles("
#include <flite/flite.h>

static int fliteAudioCb(const cst_wave *w, int start, int size,
    int last, cst_audio_streaming_info *asi)
{
    (void)w;
    (void)start;
    (void)size;
    (void)last;
    (void)asi;
    return CST_AUDIO_STREAM_STOP;
}

int main()
{
    cst_audio_streaming_info *asi = new_audio_streaming_info();
    asi->asc = fliteAudioCb; // This fails for old Flite
    new_audio_streaming_info();
    return 0;
}
" HAVE_FLITE)

cmake_pop_check_state()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLITE
    FOUND_VAR
        FLITE_FOUND
    REQUIRED_VARS
        FLITE_LIBRARY
        FLITE_INCLUDE_DIR
        HAVE_FLITE
)

if(FLITE_FOUND)
    add_library(Flite::Flite UNKNOWN IMPORTED)
    set_target_properties(Flite::Flite PROPERTIES
        IMPORTED_LOCATION "${FLITE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FLITE_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${ALSA_LIBRARIES}"
    )
endif()

mark_as_advanced(FLITE_LIBRARY FLITE_INCLUDE_DIR HAVE_FLITE)


if(HAVE_FLITE)
    set(Flite_FOUND 1)
else()
    message("Flite was found, but the version is too old (<2.0.0)")
    set(Flite_FOUND 0)
endif()
