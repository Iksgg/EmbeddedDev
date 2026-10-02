import serial
import time

ser = None

def open_serial_port(port, baudrate):
    global ser
    # Varmistetaan että baudrate on varmasti kokonaisluku eikä merkkijono
    ser = serial.Serial(str(port), int(baudrate), timeout=1)
    ser.dtr = True
    ser.rts = True

def send_and_read(data):
    global ser
    if not ser:
        raise Exception("Port not open")
    ser.write(data.encode('ascii'))
    ser.flush()
    time.sleep(0.2)
    response = ser.read_all().decode('ascii', errors='ignore')
    return response

def close_serial_port():
    global ser
    if ser:
        ser.close()