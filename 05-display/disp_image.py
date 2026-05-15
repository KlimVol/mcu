import time
import serial
from PIL import Image

SERIAL_PORT = 'COM5' 
BAUD_RATE = 115200

img_path = 'C:/Users/climr/Downloads/ant-mcu-master/ant-mcu-master/5 Как абстрагироваться от железа/Задания/pics/get.jpg'
image = Image.open(img_path)
image = image.convert('RGB')
width, height = image.size

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    for y in range(height):
        for x in range(width):
            r, g, b = image.getpixel((x, y))
            
            color_hex = f"0x{r:02X}{g:02X}{b:02X}"
            
            command = f"disp_px {x} {y} {color_hex}\n"
            
            ser.write(command.encode('utf-8'))
            
            time.sleep(0.001) 
            
            
finally:
    if ser and ser.is_open:
        time.sleep(0.1)
        ser.close()
