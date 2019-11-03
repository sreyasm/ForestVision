import serial,os
#from subprocess import call

ser = serial.Serial(
    port='COM12',
    baudrate=115200,
)
data = ser.read(153666)
print('Recieved Data!')
ser.close()
file = open('data.bmp', 'wb')
file.write(data)
file.close()
