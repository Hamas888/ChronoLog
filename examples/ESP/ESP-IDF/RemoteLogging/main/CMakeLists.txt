idf_component_register(
    SRCS "RemoteLogging.cpp"
    INCLUDE_DIRS "."
    REQUIRES ChronoLog nvs_flash esp_wifi esp_netif
)
