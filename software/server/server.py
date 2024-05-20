from flask import Flask, jsonify, render_template, request
from transceiver import get_greenhouse_sensor_data
import mysql.connector
import json

app = Flask(__name__, template_folder="templates", static_folder="static")

db = mysql.connector.connect(
    host="localhost",
    user="pi",
    password="password",
    database="main"
)

site_data_template = {
    "id": 0,
    "airHumidity": 0,
    "airTemperature": 0,
    "soilMoisture": 0,
    "dateTime": 0,
    "battery": 0
}

cursor = db.cursor()

@app.route("/")
def index():
    return render_template('index.html')

@app.route("/data", methods=["POST"])
def get_data():
    all_site_data = []
    data = request.json
    print(data)
    cursor.execute("SELECT * FROM sensori_andmed WHERE aeg >= " + str(data["startTime"]) + " AND aeg <= " + str(data["endTime"]))
    data = cursor.fetchall()
    for row in data:
        site_data = dict(site_data_template)
        site_data["id"] = row[0]
        site_data["airHumidity"] = row[3]
        site_data["airTemperature"] = row[2]
        site_data["soilMoisture"] = row[4]
        site_data["dateTime"] = row[1]
        site_data["battery"] = row[5]
        all_site_data.append(site_data)
    print("database:", json.dumps(all_site_data))
    return json.dumps(all_site_data), 200

@app.route("/get_current_data", methods=["GET"])
def get_current_data():
    get_greenhouse_sensor_data()
    return json.dumps([site_data_template]), 200

@app.route("/set_irrigation_schedule", methods=["POST"])
def set_irrigation_schedule():
    print(request.json)
    return json.dumps(request), 200

@app.route("/set_irrigation_threshold", methods=["POST"])
def set_irrigation_threshold():
    print(request.json)
    return json.dumps(request), 200


if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0')
