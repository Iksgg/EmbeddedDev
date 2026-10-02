import serial
import time

# Avaa portti samalla tavalla kuin Robot tekee
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
ser.dtr = True  # Pakotetaan DTR päälle, jos nRF vaatii sen
ser.rts = True

print("Portti auki. Laitetaan dataa...")
ser.write(b"000005\n")
ser.flush()

# Odotetaan hetki ja luetaan mitä tulee takaisin
time.sleep(0.5)
response = ser.read_all()
print(f"Raakadata saatu: {response}")

ser.close()