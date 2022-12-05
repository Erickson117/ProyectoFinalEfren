#include<16f887.h>
#include<stdbool.h>
#device ADC=10 //tamaño en bits de resolución 
#include<math.h>
#fuses INTRC_IO,NOWDT,PROTECT,NOLVP,MCLR,NOBROWNOUT
#use delay(INTERNAL=4000000)
#use RS232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, timeout=100)
#include"lcd.c"
float distancia, tiempo;
#define trig pin_A3
#define echo pin_A2
#USE standard_io(a)
#use fixed_io(b_outputs = PIN_B0, PIN_B1, PIN_B2) //B0 para el relevador, B1 para el led y b2 para el buzzer
void main(){ //MAIN INICIO
   
   //configuracion entradas analogicas
   setup_adc_ports(sAN0);   //se configura el puerto ANO como entrada analogica
   setup_adc_ports(sAN1);  //se configura el puerto AN1 como entrada analogica
   setup_adc(ADC_CLOCK_INTERNAL); //Activa todas las entradas y fuente de reloj interno
   int16 iTEMP; //variable de entrada analogica para la temperatura
   int16 iPOT; //variable de entrada analogica para la seleccion de temperatura
   int16 tempSelec; //temperatura seleccionada traducida a grados c
   int16 tempActual; //temperatura acual traducida a grados c
   float R1 = 10000; // value of R1 on board
   float logR2, R2, T;
   float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart
   int16 tempEncendido; //temperatura a la que se volvera a encender el circuito
   bool tempAlcan;   //boleano para saber si la temperatura ya llego donde debe
   char cCaracter; //caracter de entrada para la informacion que se manda desde la PC
   char cApagar;
   //configuracion pantalla lcd
   lcd_init();         //inicializacion de la lcd
   printf(LCD_PUTC, "\f Proyecto Final");
   delay_ms(1000);
   printf(LCD_PUTC, "\f Equipo 6");
   delay_ms(1000);
   //configuracion del timer
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   while(true){//WHILE PRINCIPAL INICIO
      
      while(cCaracter !=  'A'){ //WHILE ESPERA INICIO
         cApagar = 'A';
         set_adc_channel(0);     //elegimos el canal cero porque es el canal de la entrada analogica que vamos a usar 
         delay_ms(10);           //ponemos un delay de 10 ms
         iPOT = read_adc();     //realizamos una lectura del AN0 osea la entrada analogica-digital cero
         tempSelec = (iPOT*0.0586510264) + 10; //conversion de la entrada analogica a los grados que deberia
         //configuracion de la lcd para que eñl usuario vea que temperatura va a elegir
         lcd_gotoxy(1,1);
         printf(lcd_putc, "\fSeleccion de"); 
         lcd_gotoxy(1,2);
         printf(lcd_putc, "Temperatura: %Luc",tempSelec);
         cCaracter = getch();
         delay_ms(500);
      } //WHILE ESPERA FINAL
      cApagar = getch();
      if(cApagar == 'B')
      {
         cCaracter = 'B';
      }
      //printf(lcd_putc,"\f %Lu",tempSelec);
      set_adc_channel(1);     //elegimos la entrada analogica del canal 1 para guardar el dato de la resistencia
      delay_ms(10);           //delay de 10 ms
      iTEMP = read_adc();     //escribimos el valor del canal 1 en la variable iTEMP
      R2 = R1 * (1023.0 / (float)iTEMP - 1.0); //calculate resistance on thermistor
      logR2 = log(R2);
      T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
      tempActual = T - 273.15; //convert Kelvin to Celcius
      tempEncendido = tempSelec-5; //generamos la variable para saber cuando volver a encender el agua cuando se enfrie
      
      //Sensor ultrasonico 
      output_high(trig);    //mandamos la salida de la señal en alto de ultrasonido               
      delay_us(10);         //damos un dale en us              
      output_low(trig);     //mandamos una salida en bajo de la señal del ultrasonido
      while(!input(echo))                     
      {
      set_timer1(0);      //cuando el echo regrese reiniciamos el timer 
      }
      while(input(echo))                     
      {
      tiempo=get_timer1(); //obtenemos el tiempo de ida y vuelta
      }
      distancia=(tiempo/2)/(29.15); //usamos el tiempo de ida y vuelta de la señal para saber la distancia a la que estamos
      //informacion de la lcd
      printf(lcd_putc, "\f");
      lcd_gotoxy(1,1);
      //printf(lcd_putc,"Distancia =%f",distancia);
      printf(lcd_putc, "TACT = %LuC",tempActual);
      //printf("\n\rDistancia =%f",distancia);
      lcd_gotoxy(1,2);
      printf(lcd_putc, "TSEL = %LuC", tempSelec);
      delay_ms(500);
      if(tempActual >= tempSelec) //revisamos si la temperatura actual ya alcanzo o supero la temperatura que debe, en caso de que si
      { //IF1 INICIO 
         //printf("Apagar");
         output_b(0b00000010); //apagamos el relevador ya que ya no necesitamos que caliente la resistencia y encendemos el led
         tempAlcan = (tempActual <= tempEncendido); //damos un valor boleano para saber que la temperatura de reinicio no ha llegado
         int i; 
         for(i = 0; i < 4; i++) //hacemos sonar el buzer 5 veces
         { //FOR I
            output_b(0b00000110); //aqui hacemos las salidas digitales, encendemos el buzzer
            delay_ms(1000); //delay de 1 segundo
            output_b(0b00000010); //apagar el buzzer
            delay_ms(100); //delay de 100 ms
         }//FOR  F
         
      }else //IF1 FINAL
      { //ELSE1 INICIO
            
            printf("Actual: %Lu\n\r",tempActual); //mandamos informacion por puerto serie
            printf("Encendido: %Lu\n\r",tempEncendido); 
            printf("Distancia: %f\n\r",distancia);
         if(tempAlcan) //si ya se alcanzo la temperatura que debe ser para el inicio
         { //IF2 INICIO
            printf("Encender\n\r"); //informacion por puerto serie
            if(distancia > 10)  //revisamos si la distancia es mayor a dies para saber que nadie ha metido la mano
            {
               output_b(0b00000001); //encendemos el relevador y hacemos parpadear el led
               output_b(0b00000011); //
               delay_ms(1000); //delay de i segundo entre el led apagado y prendido
               output_b(0b00000001);
               delay_ms(1000);
               
            }else
            {
               output_b(0b00000000); //si alguien mete la mano el relevador se apaga
            
            }
         
         
         }else//IF2 FINAL
         {  
              tempAlcan = (tempActual <= tempEncendido); //si la temperatura actal ya alcanzo a la que necesita para encender
              output_b(0b00000000);  //apagamos todo y esperamos a que el agua se enfrie hasta el ponto de la temperatura de encendido
         }
 
      }//ELSE INICIO FINAL
  
   }//WHILE PRINCIPAL FINAL
} //MAIN FINAL
