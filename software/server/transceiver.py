import serial
import time

port = '/dev/ttyACM0'
baudrate = 115200
timeout = 2

cmd = {
    "set-irrigation-threshold": "transmit:cmd/set-irrigation-threshold",
    "set-irrigation-schedule": "transmit:cmd/set-irrigation-schedule",
    "get-irrigation-schedule": "transmit:cmd/get-irrigation-schedule",
    "get-data": "transmit:cmd/get-site-data",
    "get-battery-state": "transmit:cmd/get-battery-state",
    "set-rtc": "transmit:cmd/set-rtc",
    "ping": "transmit:cmd/ping",
}

def serial_receive(tx_cmd):
    ser = serial.Serial(port, baudrate, timeout=timeout)
    ser.flushInput()
    ser.flushOutput()
    ser.write(cmd[tx_cmd].encode("utf-8"))
    data = ""
    start_time = time.time()
    try:
        while True:
            line = ser.readline().decode().strip()
            if line:
                print("RADIO LOG:", line)
                data += line + "\n" 
            if time.time() - start_time > 2:
                break
            
    except KeyboardInterrupt:
        print("Serial port closed.")
    finally:
        ser.close()

    return data


def serial_transmit(tx_cmd):
    ser = serial.Serial(port, baudrate, timeout=timeout)
    ser.flushInput()
    ser.flushOutput()
    ser.write(tx_cmd.encode("utf-8"))
    data = ""
    start_time = time.time()
    try:
        while True:
            line = ser.readline().decode().strip()
            if line:
                print("RADIO LOG:", line)
                data += line + "\n" 
            if time.time() - start_time > 2:
                break
            
    except KeyboardInterrupt:
        print("Serial port closed.")
    finally:
        ser.close()

    return data

def get_controller_battery():
    data = serial_receive("get-battery-state")    
    data_lines = data.strip().split("\n")
    #for line in data_lines:
        #print(line.split(":"))
    if "RADIO DATA" in data_lines[-1]:
        battery_state = data_lines[-1].split(":")[-4:]
        print(battery_state)
        id = battery_state[0].split("-")[1]
        fault = battery_state[1].split("-")[1]
        chrg = battery_state[2].split("-")[1]
        voltage = battery_state[3].split("-")[1]
        return id, fault, chrg, voltage
    else: return []

def get_greenhouse_sensor_data():
    data = serial_receive("get-data")    
    data_lines = data.strip().split("\n")
    #for line in data_lines:
        #print(line.split(":"))
    if "RADIO DATA" in data_lines[-1]:
        sensors = data_lines[-1].split(":")[-3:]
        id = sensors[0].split("-")[1]
        air_humidity = sensors[1].split("-")[1]
        air_temp = sensors[2].split("-")[1]
        soil_humidity = sensors[3].split("-")[1]
        return id, air_humidity, air_temp, soil_humidity
    else: return []

def ping():
    serial_receive("ping")   

def set_irrigation_schedule(time1, duration1, time2, duration2):
    # "12:31:15:12:31:15" format hour:minutes:duration x2
    set_irrigation_cmd = cmd["set-irrigation-schedule"] + "/" + time1 + ":" + duration1 + ":" + time2 + ":" + duration2

    print(set_irrigation_cmd)
    data = serial_transmit(set_irrigation_cmd)
    data_lines = data.strip().split("\n")
    if "irrigation set" in data_lines[-1]:
        return 1
    return 0

def set_irrigation_threshold(threshold):
    # "12:31:15:12:31:15" format hour:minutes:duration x2
    set_irrigation_cmd = cmd["set-irrigation-threshold"] + "/" + str(threshold)

    print(set_irrigation_cmd)
    data = serial_transmit(set_irrigation_cmd)
    data_lines = data.strip().split("\n")
    if "irrigation set" in data_lines[-1]:
        return 1
    return 0

def sync_rtc():
    time_str = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
    rtc_cmd = cmd["set-rtc"] + "/" + time_str
    print(rtc_cmd)
    data = serial_transmit(rtc_cmd)
    data_lines = data.strip().split("\n")
    if "remotetime" in data_lines[-1]:
        return 1
    return 0


if __name__ == "__main__":
    #print(set_irrigation_threshold(500))
    #print(set_irrigation_schedule("12:14", "15", "9:30", "10"))
    #print(sync_rtc())
    #print(get_greenhouse_sensor_data())
    print(get_controller_battery())
    """
    while(True):
        ping()
        time.sleep(5)
    """
    
