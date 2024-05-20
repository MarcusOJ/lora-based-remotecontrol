

#set_irrigation_schedule:

request body 
```
{ time: '20:00', length: '33' } 

```


#set_irrigation_threshold:

request body
```
{ threshold: '33' }

```


#get_current_data:

response
```
 [
      {
        "airHumidity": 1,
        "airTemperature": 20,
        "soilHumidity": 2000,
        "dateTime": 1620000000,
        "battery": 100,
        "id": 1,
      },
      {
        "airHumidity": 50,
        "airTemperature": 1,
        "soilHumidity": 1,
        "dateTime": 1620000000,
        "battery": 90,
        "id": 2,
      }
    ]

```