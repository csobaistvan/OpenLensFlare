# Get the base file name component
get_filename_component(SHADER_BASE_NAME "${INPUT_FILE_NAME}" NAME_WE)

# Write out the file contents
file(WRITE "${OUTPUT_HEADER_NAME}" "")
file(APPEND "${OUTPUT_HEADER_NAME}" "#pragma once\n")
file(APPEND "${OUTPUT_HEADER_NAME}" "namespace OLEF{ namespace Shaders{\n")
file(APPEND "${OUTPUT_HEADER_NAME}" "extern const char* ${SHADER_BASE_NAME};\n")
file(APPEND "${OUTPUT_HEADER_NAME}" "}}")

# Load the shader file
file(READ "${INPUT_FILE_NAME}" SHADER_CONTENTS)

# Write out the file contents
file(WRITE "${OUTPUT_SOURCE_NAME}" "")
#file(APPEND "${OUTPUT_SOURCE_NAME}" "#include \"${SHADER_BASE_NAME}.h\"\n")
file(APPEND "${OUTPUT_SOURCE_NAME}" "namespace OLEF{ namespace Shaders{\n")
file(APPEND "${OUTPUT_SOURCE_NAME}" "extern const char* ${SHADER_BASE_NAME} = R\"SHADER(\n")
file(APPEND "${OUTPUT_SOURCE_NAME}" "${SHADER_CONTENTS}")
file(APPEND "${OUTPUT_SOURCE_NAME}" ")SHADER\";\n")
file(APPEND "${OUTPUT_SOURCE_NAME}" "}}")