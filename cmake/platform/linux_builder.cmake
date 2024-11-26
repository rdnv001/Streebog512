include(cmake/utils/get_linux_kernel.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
    -fvisibility=hidden
    -pedantic
    -Wall
    -Wextra
    -Wcast-align
    -Wcast-qual
    -Wctor-dtor-privacy
    -Wformat=2
    -Winit-self
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wnoexcept
    -Woverloaded-virtual
    -Wredundant-decls
    -Wshadow
    -Wsign-promo
    -Wstrict-null-sentinel
    -Wstrict-overflow=5
    -Wswitch-default
    -Wundef
    -Wno-unused-variable
    -Wno-error=redundant-decls
    -Ofast
)

include(cmake/platform/library_build.cmake)

# Add executable
add_executable(${PROJECT_NAME} ${STREEBOG_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${STREEBOG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${STREEBOG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SYSTEM_ENCRYPTOR_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)