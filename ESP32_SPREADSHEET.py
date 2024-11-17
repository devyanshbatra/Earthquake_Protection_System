import machine
import time
import urequests  
ssid = "ZEUS"
password = "Devyansh07"
host = "https://docs.google.com/spreadsheets/d/1gxq8_La4ZqfiMdbNG7IpxSnD-VQiOy5Qlt3SuReaXWc/edit?gid=0#gid=0"
sensor_pin = machine.ADC(5)  
def connect_wifi():
  """Connects to the WiFi network."""
  wifi = machine.WLAN(mode=machine.WLAN.STA)
  if not wifi.isconnected():
    print("Connecting to WiFi...")
    wifi.connect(ssid, password)
    while not wifi.isconnected():
      time.sleep(0.5)
      print(".")
  print("WiFi connected")

def send_data(sensor_value):
  """Sends sensor data to Google Apps Script."""
  url = host
  headers = {"Content-Type": "application/x-www-form-urlencoded"}
  data = {"sensorReading": str(sensor_value)}
  try:
    response = urequests.post(url, headers=headers, data=data)
    if response.status_code == 200:
      print("Data sent successfully")
    else:
      print(f"Error sending data: {response.status_code}")
  except Exception as e:
    print(f"Error: {e}")
  finally:
    response.close() 

def main():
  """Main loop for reading sensor data and sending it to Google Apps Script."""
  connect_wifi()
  while True:
    sensor_value = sensor_pin.read_analog()  
    send_data(sensor_value)
    time.sleep(1)  
if __name__ == "__main__":
  main()