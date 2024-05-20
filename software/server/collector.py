import mysql.connector
from transceiver import get_greenhouse_sensor_data, get_controller_battery
import time

db = mysql.connector.connect(
    host="localhost",
    user="pi",
    password="password",
    database="main"
)

cursor = db.cursor()

if __name__ == "__main__":
    try:
        id = get_controller_battery()[0]
        battery = get_controller_battery()[-1]
        air_humidity = get_greenhouse_sensor_data()[0]
        air_temp = get_greenhouse_sensor_data()[1]
        soil_humidity = get_greenhouse_sensor_data()[2]
    except Exception as e:
        print(e)
        
    datetime = time.time()
    sql = "INSERT INTO sensor_data (seadme_id, aeg, ohutemperatuur, ohuniiskus, pinnaseniiskus, akupinge) VALUES (%s, %s, %s, %s, %s, %s)"
    val = (id, datetime, air_humidity, air_temp, soil_humidity, battery)
    
    cursor.execute(sql, val)
    db.commit()

    print(cursor.rowcount, "records inserted.")

    