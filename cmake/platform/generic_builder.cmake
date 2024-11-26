message(STATUS "CXX compiler:      ${CMAKE_CXX_COMPILER_ID}")

if(NOT CMAKE_RELEASE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# [SOURCE DIRECTORIES]
set(STREEBOG_SRC_DIR "${PROJECT_SOURCE_DIR}/Streebog512Prog/src/")


# [INCLUDE DIRECTORIES]
set(STREEBOG_INCLUDE_DIRS
)

# [SOURCE FILES]
file(GLOB STREEBOG_SRC CONFIGURE_DEPENDS
    "${STREEBOG_SRC_DIR}/*.hpp"
    "${STREEBOG_SRC_DIR}/*.cpp"
)

if(MACOS)
    #include(cmake/platform/macos_builder.cmake)
elseif(LINUX)
    include(cmake/platform/linux_builder.cmake)
elseif(WIN32)
    #include(cmake/platform/windows_builder.cmake)
else()
    message(WARNING "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
    include(cmake/platform/linux_builder.cmake)
endif()

# [SOURCE GROUPS]
source_group("Encryptor Base"  FILES ${STREEBOG_SRC})
