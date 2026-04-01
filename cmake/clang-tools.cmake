# Adapted from: https://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html

file(GLOB_RECURSE
     ALL_SOURCE_FILES
     ${CMAKE_SOURCE_DIR}/src/*.h
     ${CMAKE_SOURCE_DIR}/src/*.cpp
     ${CMAKE_SOURCE_DIR}/src/*.inl
    )

find_program(CLANG_FORMAT "${CLANG_TOOLS_PATH}/clang-format")
if(CLANG_FORMAT)
    add_custom_target(
        CLANG-FORMAT
        COMMAND "${CLANG_TOOLS_PATH}/clang-format"
        -i
        -style=file
        ${ALL_SOURCE_FILES}
    )
endif()