cmake_minimum_required(VERSION 3.27)
project(Database)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 10.1.1
)

FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
)

FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(sfml)

add_executable(Database main.cpp
        Database.cpp
        Database.h
        DBQLParser.cpp
        DBQLParser.h
        PreRequistion.h
        WindowManager.cpp
        WindowManager.h


)
target_link_libraries(
        Database
        fmt
        sfml-graphics
        sfml-window
        sfml-system
)

IF (WIN32)
    add_custom_command(TARGET Database POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Database> $<TARGET_FILE_DIR:Database>
            COMMAND_EXPAND_LISTS
    )
ENDIF ()