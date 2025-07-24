import serial
import time
import numpy as np
from pathlib import Path

t = time.localtime()
# Random number generator
np.random.seed(int(t.tm_sec))

# Configure serial port settings
# Replace 'COM3' with your actual serial port (e.g., '/dev/ttyUSB0' on Linux)
ser = serial.Serial() 
ser.port = '/dev/cu.usbmodem11303' 
ser.baudrate = 115200
ser.timeout = 1

# Attempt to connect to the serial port
try:
    ser.open()
except Exception as e:
    print("ERROR:", e)
    exit()
    
# File Settings   
movement_name = "serpensortia" 
# Open a file to save the data
filepath = f"collected_data/{movement_name}/{movement_name}"

fileCounter = 0
# Main data collection loop
try:
    
    while True:    
        fileCounter = fileCounter + 1
        fileId = np.random.randint(0, 1000)
        filename = filepath + f"_{fileId}.csv" 
        isFile = Path(filename)

        if isFile.is_file():
            fileId = np.random.randint(0, 1000)
            filename = filepath + f"_{fileId}.csv" 
            
        with open(filename, 'a') as f:
            print(f"Saving serial data to {filename}. Press Ctrl+C to stop. File Counter: {fileCounter}")
            
            f.write("Time, AccX, AccY, AccZ, GyroX, GyroY, GyroZ" + '\n')
            
            while True:
                if ser.in_waiting > 0:
                    # Read a line from the serial port, decode it, and strip whitespace
                    try: 
                        line = ser.readline().decode('utf-8').strip()
                        
                        if "End Data Collection" in line:
                            print("File Closed")
                            f.close()
                            break
                        if line:
                            # Print the data to the console (optional)
                            print(line)
                            # Write the data to the file, followed by a newline
                            f.write(line + '\n')
                        
                    except Exception as e:
                        print(f"Error reading serial data: {e}")
                
except KeyboardInterrupt:
    print("\nData collection stopped by user.")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    # Close the serial port when done
    ser.close()
    print("Serial port closed.")
