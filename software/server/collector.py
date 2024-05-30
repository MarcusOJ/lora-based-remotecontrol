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
        id, fault, chrg, voltage = get_controller_battery()
        id, air_humidity, air_temp, soil_humidity = get_greenhouse_sensor_data()
    except Exception as e:
        print(e)
        
    datetime = time.time()
    sql = "INSERT INTO sensor_data (seadme_id, aeg, ohutemperatuur, ohuniiskus, pinnaseniiskus, akupinge) VALUES (%s, %s, %s, %s, %s, %s)"
    val = (id, datetime, air_humidity, air_temp, soil_humidity, voltage)
    
    cursor.execute(sql, val)
    db.commit()

    print(cursor.rowcount, "records inserted.")

    