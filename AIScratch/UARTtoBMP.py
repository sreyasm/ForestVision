import serial,os
#from subprocess import call

ser = serial.Serial(
    port='COM12',
    baudrate=115200,
)
#data = ser.read(30054)
#print('Recieved Data!')
#file = open('RGB888.bmp', 'wb')
#file.write(data)

data = ser.read(153666)
print('Recieved Data!')
ser.close()
file = open('RGB565.bmp', 'wb')
file.write(data)
file.close()
