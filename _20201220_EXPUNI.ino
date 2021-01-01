

/************************************************************************
 *          PROGRAMA PLC DE CONTROL DE LA PLANTA
 *          DE ESTERILIZACIÓN 
 *          PLC-UV-H²O
 * **********************************************************************
 */



#include <Servo.h>    // Librería control servo
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


// Declaración de los servos
// NOTA: servo rotación continua: ángulo 0 dirección izq, ángulo 180 dirección derecha
Servo servorampa;     // servo tracción rampa elevadora
Servo servocinta;     // servo tracción cinta transportadora
int velrampa = 90;    // velocidad de la rampa elevadora = 0
int velcinta = 90;    // velocidad de la cinta transportadora = 0
Servo brazoelevador;  // servo brazo elevador
int brazoabajo;     // Posición inicial del brazo
int brazoarriba;       // Posición final del brazo

// Sensor IR para piezas en la cinta transportadora
const int sensorPin = 2; // Pin donde se conecta el sensor IR
int medida;              //Medición del sensor
int sumatorio;           //Sumatorio de detecciones del sensor

char command;   //Comando recibido por el PLC-Arduino

// I2C pantalla LCD declaración de pins
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

/*****************************************************
 * INICIALIZACIÓN DE VARIABLES
 * 
 */

void setup() {

    Serial.begin(9600);       // Monitor serie
    
    servorampa.attach(9);     // servo rampa elevadora
    servocinta.attach(10);    // servo cinta transportadora
    brazoelevador.attach(11); // servo brazo elevador

    brazoabajo = 177;
    brazoarriba = 90;
    brazoelevador.write(brazoabajo); // ponemos el brazo elevador en su posición inicial

    // Entrada del sensor IR
    pinMode(sensorPin , INPUT);  //definir pin como entrada
    sumatorio = 0;

    // Led que simula la luz ultravioleta
    pinMode(12, OUTPUT);    // configura pin 12 como salida

    // Inicialización de pantalla LCD 16x2, encendida
    lcd.begin(16,2);
    lcd.backlight();
    // Salida por LCD
   lcd.setCursor(0,0);
   lcd.print("PLC-UV-H2O");
   lcd.setCursor(0,1);
   lcd.print("----v1.0");
}


/*****************************************************
 * BUCLE DE CONTROL PRINCIPAL
 * 
 */
void loop() {

  // Leemos del puerto serie los comandos si hay alguno disponible
  if(Serial.available()) {
    command = Serial.read();
  
    Serial.print("Comando: ");
    Serial.println(command);


   
    // Planta en CICLO DE FUNCIONAMIENTO CONTINUO
    if (command == 'I'){
      // Primero inicializamos variables de entorno correctamente
      inicializarvariables();
    }
    if(command == 'C'){
      // Segundo ponemos en marcha el circuito
      funcionamientoContinuo();
      // Control del funcionamiento por monitor serie PC arduino
      salidaMonitorSerie();  
    }  

    // Planta en MODO MANTENIMIENTO
    if(command == 'M'){
      // Primero detenemos el circuito
      detenercircuito();
      delay(1000); 
    }  
    if(command == 'B'){
      // Segundo elevamos el brazo y modificando la variable de entorno
      // para evitar accidentes
      brazoabajo = 90;
      brazoarriba = 90;
      brazoelevador.write(brazoarriba);
      
      // Salida por LCD
      lcd.setCursor(0,0);
      lcd.print("PLC-UV-H2O");
      lcd.setCursor(0,1);
      lcd.print("***MANTENIMIENTO ");
    }  

  }// -(if serial)  

}

/*****************************************************
 * FUNCIÓN INICIALIZAR VARIABLES DE ENTORNO
 */
void inicializarvariables()
{
    // Definimos el ángulo límite del movimiento del brazo de recogida de envases
    // tanto inferior como superior
    brazoabajo = 177;
    brazoarriba = 90;
    brazoelevador.write(brazoabajo); // ponemos el brazo elevador en su posición inicial
}
/*****************************************************
 * FUNCIÓN CIRCUITO FUNCIONAMIENTO CONTINUO
 */
void funcionamientoContinuo()
{
   bool salir = false;
   while (!salir){
   // rampa en marcha sentido hacia arriba
   velrampa = 80;
   servorampa.write(velrampa);                 

   //cinta transportadora sentido derecha
   velcinta = 98;
   servocinta.write(velcinta);              

   // Detección de objeto en la cinta transportadora

  medida = digitalRead(sensorPin );  //lectura del sensor IR
  if (medida == LOW)
    {
      sumatorio = sumatorio +1;
      }

  // Salida por LCD
  lcd.setCursor(0,0);
  lcd.print("PLC-UV-H2O");
  lcd.setCursor(0,1);
  lcd.print("**CICLO CONTINUO");
  // Control por salida monitor serie arduino
  Serial.print(" --Sumatorio: ");
  Serial.println(sumatorio);

  if (sumatorio > 25)// Umbra nº de detecciones del sensor IR.
    {
      devolverpiezaalcircuito();
      }

   // Leemos el puerto serie, si hay comando de interrupción de ciclo salimos
   if(Serial.available()) {
    command = Serial.read();
   }
   if (command == 'X'){ // 'X' comando de salida EXIT
    salir = true;
    detenercircuito();
   } 
   }//--(while(salir))
  }


/*****************************************************
 * FUNCIÓN DETENER CIRCUITO
 */
void detenercircuito()
{
    // Detenemos la rampa y la cinta, si hay carga en el brazo la situamos al inicio del circuito
    int velrampa = 90;    // velocidad de la rampa elevadora = 0
    servorampa.write(velrampa); 
    int velcinta = 90;    // velocidad de la cinta transportadora = 0
    servocinta.write(velcinta);
    brazoelevador.write(brazoarriba); // ponemos el brazo elevador en su posición inicial
    delay(1500);
    brazoelevador.write(brazoabajo); // ponemos el brazo elevador en su posición inicial

    // Salida por LCD
    lcd.setCursor(0,0);
    lcd.print("PLC-UV-H2O");
    lcd.setCursor(0,1);
    lcd.print("**STOP STOP STOP");
  }

  
/*****************************************************
 * FUNCIÓN DEVOLUCIÓN PIEZA AL CIRCUITO
 */
void devolverpiezaalcircuito()
  {

  // Salida por LCD
  lcd.setCursor(0,0);
  lcd.print("PLC-UV-H2O");
  lcd.setCursor(0,1);
  lcd.print("****RECICLADO   ");

  // Enciende la luz "ultravioleta"
   digitalWrite(12, HIGH);
  
   //paramos la rampa
   velrampa = 90;
   servorampa.write(velrampa); 
   //cinta transportadora unos segundos funcionando
   velcinta = 98;
   servocinta.write(velcinta);   
   delay(3000);
   //paramos la cinta
   velcinta = 90;
   servocinta.write(velcinta); 
   //ponemos en marcha la rampa de nuevo
   velrampa = 80;
   servorampa.write(velrampa); 
   delay(1000);
   
   // Apaga la luz "ultravioleta"
   digitalWrite(12, LOW);
   
   //movemos el brazo elevador para devolver la pieza al circuito
   brazoelevador.write(brazoarriba);
   delay(1500);
   brazoelevador.write(brazoabajo);
   //ponemos el contador del detector a cero
   delay(500);
   sumatorio = 0;

  }


/*****************************************************
 * FUNCIÓN SALIDA MONITOR SERIE ARDUINO
 */
void salidaMonitorSerie()
{
    // **** SALIDA POR EL MONITOR SERIE PARA CONTROL
  Serial.println("-");  
  Serial.print("Comando introducido: ");
  Serial.print(command); 
  Serial.print("-Ping: ");
  Serial.print(medida); 
  Serial.print(" cm. -Sumatorio: ");
  Serial.println(sumatorio);   
   }
