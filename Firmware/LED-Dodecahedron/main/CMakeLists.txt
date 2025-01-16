idf_component_register(
    SRCS "main.c"

        "Power/power.c"
        "Power/fuel_gauge.c"
        "Power/power_meter.c"
    
        "Temperature/temperature_sensor.c"
    
        "LED_Panels/led_panels.c"

        "Network/http_server.c"
        "Network/network.c"

        "effects/crazybees.c"
        "effects/dna.c"
        "effects/fire.c"
        "effects/matrix.c"
        "effects/noise.c"
        "effects/plasma_waves.c"
        "effects/rain.c"
        "effects/rainbow.c"
        "effects/rays.c"
        "effects/sparkles.c"
        "effects/waterfall.c"
        "effects/solid_color.c"
    INCLUDE_DIRS "."    
    EMBED_FILES "Network/app_index.html"
)