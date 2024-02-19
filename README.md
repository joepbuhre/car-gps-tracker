# Car GPS tracker

### Example use in Home Assistant

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

### TODOS:
- Battery management is not as good
