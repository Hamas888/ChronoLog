aux_source_directory(src SOURCES)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include"
)
