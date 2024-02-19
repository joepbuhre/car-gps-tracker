# Car GPS tracker

### Example use in Home Assistant

#### Put below in configuration.yaml
> Do note the peugeot307, didn't change that one because of it's currently hardcoded
```
mqtt:
  sensor:
    - name: "Car"
      unique_id: car_tracker
      state_topic: "home/device_tracker/peugeot307/state"
      value_template: '{{ value_json["state"] }}'
      json_attributes_topic: "home/device_tracker/peugeot307/attributes"
      json_attributes_template: "{{ value_json | tojson }}"
      
    - name: "Car Location updated"
      device_class: timestamp
      unique_id: car_tracker_updated
      state_topic: "home/device_tracker/peugeot307/state"
      value_template: "{{ now() }}"
      # json_attributes_topic: "{{value_json | to_json}}"
    
    - name: "Car Tracker on Battery"
      device_class: voltage
      unique_id: car_tracker_on_battery
      state_topic: "home/device_tracker/peugeot307/battery"
      value_template: "{{value_json.state}}"
```

#### Below is an example dashboard with mushroom:
```
views:
  - title: Home
    cards:
      - type: map
        entities:
          - entity: sensor.car
        hours_to_show: 10
      - type: custom:layout-card
        layout_type: custom:grid-layout
        layout:
          grid-template-columns: 50% 50%
        cards:
          - type: custom:mushroom-template-card
            primary: Auto
            secondary: >-
              {% if states('binary_sensor.car_tracker_on_battery') ==
              'on' %}Onderweg ({{state_attr('sensor.car','speed') |
              round(2)}} km/h){%else%}Gestopt{%endif%}
            icon: >-
              {% if states('binary_sensor.car_tracker_on_battery') ==
              'on' %}mdi:car{%else%}mdi:car-off{%endif%}
            badge_color: ''
            icon_color: >-
              {% if states('binary_sensor.car_tracker_on_battery') ==
              'on' %}teal{%else%}amber{%endif%}
            multiline_secondary: true
            entity: sensor.car
            tap_action:
              action: more-info
          - type: custom:mushroom-template-card
            primary: Snelheid
            secondary: '{{state_attr(''sensor.car'',''speed'') | round(2)}}km/h'
            icon: >-
              {% if (state_attr('sensor.car','speed') | round(2)) > 80
              %}
                mdi:speedometer
              {% elif state_attr('sensor.car','speed') | round(2) > 40
              %}
                mdi:speedometer-medium
              {% else %}
                mdi:speedometer-slow
              {% endif %}
            icon_color: >-
              {% if (state_attr('sensor.car','speed') | round(2)) > 80
              %}
                green
              {% elif state_attr('sensor.car','speed') | round(2) > 40
              %}
                yellow
              {% else %}
                red
              {% endif %}
          - type: custom:mushroom-template-card
            primary: >-

              {% if states('sensor.car_tracker_on_battery') | round(2) >
              0 %}
                Batterij 
              {% else %}
                Aan het opladen
              {% endif %}
            secondary: >-
              {% set vlt = states('sensor.car_tracker_on_battery') |
              round(2) %}


              {% if states('sensor.car_tracker_on_battery') | round(2) >
              0 %}
                {{states('sensor.car_tracker_on_battery') | round(2)}} volt ({{ (((vlt - 3250) / (4160 - 3250)) * 100) | round(2) }}%)
              {% endif %}
            icon: >-
              {% if states('sensor.car_tracker_on_battery') | round(2) >
              0 %}
                mdi:battery-60
              {% else %}
                mdi:battery-charging
              {% endif %}
            icon_color: >-
              {% if states('sensor.car_tracker_on_battery') | round(2) >
              0 %}
                yellow
              {% else %}
                green
              {% endif %}
            multiline_secondary: true
          - type: custom:mushroom-template-card
            primary: >-
              Verbonden met {{state_attr('sensor.car',
              'connection_mode') }}
            secondary: ''
            icon: >
              {% if state_attr('sensor.car', 'connection_mode') ==
              'WiFi' %}
                mdi:wifi
              {% else %}
                mdi:signal-4g
              {% endif %}
            icon_color: blue
            badge_icon: ''
          - type: custom:mushroom-template-card
            primary: GPS Verbonden
            secondary: >-
              {{state_attr('sensor.car', 'used_satellites') }} gebruikt 

              ({{state_attr('sensor.car', 'visible_satellites') }}
              totaal)
            icon: |-
              {% if states('sensor.car') != 'gps_unavailable' %}
                mdi:satellite-variant 
              {% endif %}
            badge_icon: |
              {% if states('sensor.car') == 'gps_unavailable' %}
                mdi:alert-octagon
              {% endif %}
            icon_color: ''
            multiline_secondary: true
          - type: custom:mushroom-template-card
            primary: |-
              {% if states('sensor.car') == 'deep_sleep' %}
                Deep Sleep
              {% elif states('sensor.car') == 'gps_connected' %}
                Actief
              {% elif states('sensor.car') == 'low_battery' %}
                Low battery
              {% else %}
                Onbereikbaar!
              {% endif %}
            secondary: |
              {% if states('sensor.car') == 'deep_sleep' %}
                {{(as_timestamp(states('sensor.car_location_updated')) + (60 * 15)) | timestamp_custom('%Y-%m-%d %H:%M') }} 
              {% endif %}
            icon: |-
              {% if states('sensor.car') == 'deep_sleep' %}
                mdi:sleep
              {% elif states('sensor.car') == 'gps_connected' %}
                mdi:sync
              {% elif states('sensor.car') == 'low_battery' %}
                mdi:battery-10
              {% else %}
                mdi:alert
              {% endif %}
            icon_color: |-
              {% if states('sensor.car') == 'deep_sleep' %}
                indigo
              {% elif states('sensor.car') == 'gps_connected' %}
                green
              {% elif states('sensor.car') == 'low_battery' %}
                indigo
              {% else %}
                red
              {% endif %}
            badge_icon: ''
            multiline_secondary: true
            fill_container: false
          - type: custom:mushroom-template-card
            primary: Laatste Update
            secondary: >-
              {{as_timestamp(states('sensor.car_location_updated')) |
              timestamp_custom('%Y-%m-%d %H:%M')}}
            icon: mdi:clock
            badge_icon: |
              {% if states('sensor.car') == 'gps_unavailable' %}
                mdi:alert-octagon
              {% endif %}
            icon_color: ''
            multiline_secondary: true
          - type: custom:config-template-card
            variables:
              MAIN: states['sensor.car'].state
              ATTRS: states['sensor.car'].attributes
            entities:
              - sensor.car
            card:
              type: custom:mushroom-template-card
              primary: Find Peugeot
              secondary: ''
              icon: mdi:map-plus
              icon_color: brown
              tap_action:
                action: url
                url_path: >-
                  ${['https://www.google.com/maps/dir/?api=1&destination=',ATTRS.latitude,',',
                  ATTRS.longitude].join('')}
          - type: custom:mushroom-template-card
            primary: Hoogte
            secondary: '{{ state_attr(''sensor.car'', ''altitude'') }} meter'
            icon: mdi:altimeter
            badge_icon: |
              {% if states('sensor.car') == 'gps_unavailable' %}
                mdi:alert-octagon
              {% endif %}
            icon_color: brown
            multiline_secondary: true
  - theme: Backend-selected
    title: Stats
    path: stats
    icon: mdi:chart-line
    badges: []
    cards: []
```


### TODOS:
- Battery management is not as good
