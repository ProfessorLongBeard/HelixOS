add_custom_target(setup
    COMMAND
        ${CMAKE_COMMAND} -E env "$ENV{HELIX_SCRIPTS_DIR}/install.sh"

    USES_TERMINAL
)

add_custom_target(run
    COMMAND
        ${CMAKE_COMMAND} -E env "$ENV{HELIX_SCRIPTS_DIR}/run.sh"

    USES_TERMINAL
)

add_custom_target(debug
    COMMAND
       ${CMAKE_COMMAND} -E env "$ENV{HELIX_SCRIPTS_DIR}/run_debug.sh"

    USES_TERMINAL
)