# additional target to perform cppcheck run, requires cppcheck

# get all project files
# HACK this workaround is required to avoid qml files checking ^_^
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.hpp)

add_custom_target(
        cppcheck
        COMMAND cppcheck
        -I ${CMAKE_CURRENT_SOURCE_DIR}/src
        --enable=warning,style,performance,portability,information,missingInclude
        --std=c++14
        --template="[{severity}][{id}] {message} {callstack} \(On {file}:{line}\)"
        --verbose
        --quiet
        --config-exclude=${CMAKE_CURRENT_SOURCE_DIR}/src/parser/config.hpp

        ${ALL_SOURCE_FILES}
)

# --check-config