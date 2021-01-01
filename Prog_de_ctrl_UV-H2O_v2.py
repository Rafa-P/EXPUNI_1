#
# PROGRAMA DE CONTROL DE 'ESTERILIZADORA POR LOTES'
# CTRL-UV-H2O
#

import serial
import time
import os

arduino = serial.Serial('/dev/ttyUSB0',9600)

os.system("clear")

print("###########################################")
print("#      ESTERILIZACION U/V POR LOTES       #")
print("#             Control Program             #")
print("###########################################")
while True:
      comando = raw_input('Introducir comando: ')
      # COMANDO DE CICLO CONTINUO
      if comando == 'CICLO':
            arduino.write('I')
            time.sleep(.300)      
            arduino.write('C')
            time.sleep(.300)
            print('   ****ESTERILIZACION UV-H2O EN CIRCUITO CONTINUO')
            
            while True:
                comando = raw_input('Introducir comando: ')
                # COMANDO DE PARADA 
                if comando == 'STOP':
                    arduino.write('X')
                    time.sleep(.300)
                    print('   ****PARADA DE PLANTA')
                    break;
            
      # COMANDO DE MANTENIMIENTO 
      elif comando == 'MANTENIMIENTO':
            arduino.write('M')
            time.sleep(.300)
            arduino.write('B')
            time.sleep(.300)
            print('   ****PARADA DE MANTENIMIENTO')
            print('   ****BRAZO ELEVADO PARA MANTENIMIENTO')

arduino.close()

