idf_component_register(
    SRCS "main.cpp" "can_capture.cpp" "haltech_protocol.cpp" "monitor.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_twai esp_timer esp_system freertos log
)
