#define MQ2 A4

#define         RL_VALOR             (5)          //define el valor de la resistencia mde carga en kilo ohms
#define         RAL       (9.83)                  // resistencia del sensor en el aire limpio / RO, que se deriva de la tabla de la hoja de datos
#define         GAS_LP                      (0 )
String inputstring = "";                                                        //Cadena recibida desde el PC
float           LPCurve[3]  =  {4.4,3.9,2.4};                                   //Curva de resistencia para CO2 
float           Ro           =  10;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Calibrando...\n");
  Ro = Calibracion(MQ2);                        //Calibrando el sensor. Por favor de asegurarse que el sensor se encuentre en una zona de aire limpio mientras se calibra
  
  }

void loop() {
  // put your main code here, to run repeatedly:
  Nivel_ppm(MQ2);
}
void Nivel_ppm(float MQ){
   Serial.print("LP:");
   float Read_Value = lecturaMQ(MQ);
   Serial.print(Read_Value);
   Serial.print(" ");
   Serial.print(porcentaje_gas(Read_Value/Ro,GAS_LP) );
   Serial.print( "ppm" );
   Serial.print("    ");
   Serial.print("\n");
   delay(200);
}
/////////////////SUBRUTINAS////////////////
float calc_res(int raw_adc)
{
  return ( ((float)RL_VALOR*(1023-raw_adc)/raw_adc));
}
 
float Calibracion(float mq_pin){
  delay(1500);
  int i;
  float val=0;
    for (i=0;i<50;i++) {                                                                               //tomar múltiples muestras
    val += calc_res(analogRead(mq_pin));
    delay(500);
  }
  val = val/50;                                                                                         //calcular el valor medio
  val = val/RAL;
  return val;
}
 
float lecturaMQ(float mq_pin){
  int i;
  float rs=0;
  for (i=0;i<5;i++) {
    rs += calc_res(analogRead(mq_pin));
    delay(50);
  }
//rs = rs/5;
return rs;
}
 
int porcentaje_gas(float rs_ro_ratio, int gas_id){
   if ( gas_id != GAS_LP ) {
     return porcentaje_Gas(rs_ro_ratio,LPCurve);
   }
  return 0;
}
 
int porcentaje_Gas(float rs_ro_ratio, float *pcurve){
  return (pow(10, (((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
