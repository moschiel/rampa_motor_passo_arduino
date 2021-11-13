#define PUL 7 //define Pulse pin
#define DIR 6 //define Direction pin
#define ENA 5 //define Enable Pin

#define MOTOR_STEPS 200
#define RPM_UPDATE_RATE (unsigned long)50 //ms, a cada X milisegundos, o RPM é atualizado durante a rampa
#define HORARIO       0
#define ANTIHORARIO   1


void setup() {
  Serial.begin(9600);
  
  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  pinMode (ENA, OUTPUT);

  runRampa(10, 200, 10, 4, HORARIO); //gira com rampa de 10 segundos, iniciando em 10 RPM, e terminando em 200 RPM (acelerando)
  runRampa(200, 10, 10, 4, HORARIO); //gira com rampa de 10 segundos, iniciando em 200 RPM, e terminando em 10 RPM (desacelerando)
  //runRPM(60, HORARIO); //gira constante no RPM escolhido
}

void loop() {
}

unsigned long int getDelayPulso(float rpm, unsigned int microStep){
  unsigned long int pulsosPorVolta = MOTOR_STEPS * microStep;
  unsigned long int usPorVolta = (unsigned long int)(((unsigned long int)60*1000*1000)/rpm);
  unsigned long int usPorPulso = usPorVolta/pulsosPorVolta;
  //Serial.println("PULSOS POR VOLTA: " + String(pulsosPorVolta));
  //Serial.println("TEMPO POR VOLTA: " + String((unsigned long int)usPorVolta/1000) + " ms");
  //Serial.println("TEMPO POR PULSO: " + String((unsigned long int)usPorPulso) + " us");
  return usPorPulso/(float)2;
}

float getRPMincrement(unsigned int inicioRPM, unsigned int finalRPM, unsigned int tempoRampa){
  unsigned int deltaRPM = abs((int)finalRPM - (int)inicioRPM);
  double rpmPorMs = ((double)deltaRPM)/((double)tempoRampa*(double)1000);
  float rpmIncrement = rpmPorMs * (float)RPM_UPDATE_RATE;
  Serial.println("RAMPA: Incremento de " + String(rpmIncrement) + " RPM a cada " + String(RPM_UPDATE_RATE) + " ms");
  return rpmIncrement;
}

void runRampa(unsigned int inicioRPM, unsigned int finalRPM, unsigned int tempoRampa, unsigned int microStep, int sentido){
  float rpmIncrement = getRPMincrement(inicioRPM, finalRPM, tempoRampa);

  if(finalRPM > inicioRPM) //rampa acelerando
  {
    Serial.println("rampa acelerando");
    float currentRPM = (float)inicioRPM;
    while(currentRPM <= (float)finalRPM){ //fica nesse loop até chegar ao RPM final
      Serial.println("RPM: " + String(currentRPM));
      unsigned long int usPulseDelay = getDelayPulso(currentRPM, microStep);
      unsigned long initTime = millis();
      
      while((millis()-initTime) < RPM_UPDATE_RATE){ //fica nesse loop pelo tempo minimo do RPM_UPDATE_RATE
        if(sentido == HORARIO)
          giraHorario(usPulseDelay);
        else
          giraAntiHorario(usPulseDelay);
      }
      currentRPM += rpmIncrement; //incrementa RPM 
  
    }
  }
  else 
  {
    Serial.println("rampa desacelerando");
    float currentRPM = (float)inicioRPM;
    while(currentRPM > (float)finalRPM){ //fica nesse loop até chegar ao RPM final
      Serial.println("RPM: " + String(currentRPM));
      unsigned long int usPulseDelay = getDelayPulso(currentRPM, microStep);
      unsigned long initTime = millis();
      
      while((millis()-initTime) < RPM_UPDATE_RATE){ //fica nesse loop pelo tempo minimo do RPM_UPDATE_RATE
        if(sentido == HORARIO)
          giraHorario(usPulseDelay);
        else
          giraAntiHorario(usPulseDelay);
      }
      currentRPM -= rpmIncrement; //diminui RPM 
    }
  }
}

void runRPM(unsigned int rpm, unsigned int microStep, int sentido){
  unsigned long int usPulse = getDelayPulso((float)rpm, microStep);
  while(1){
    if(sentido == HORARIO)
      giraHorario(usPulse);
    else
      giraAntiHorario(usPulse);
  }
}

void giraHorario(unsigned long int usPulseDelay){
  //habilita sentido horario
  digitalWrite(DIR,LOW);
  digitalWrite(ENA,HIGH);
  //pulso horario
  digitalWrite(PUL,HIGH);
  delayMicroseconds(usPulseDelay);
  digitalWrite(PUL,LOW);
  delayMicroseconds(usPulseDelay);
}

void giraAntiHorario(unsigned long int usPulseDelay){
  //habilita sentido anti horario
  digitalWrite(DIR,HIGH);
  digitalWrite(ENA,HIGH);
  //pulso
  digitalWrite(PUL,HIGH);
  delayMicroseconds(usPulseDelay);
  digitalWrite(PUL,LOW);
  delayMicroseconds(usPulseDelay);
}


/*
void checkSerial() {
  if(Serial.available() > 0){
    String rcvString = Serial.readString();

    if(rcvString.startsWith("MICRO_STEP")) {
      userConfig.microStep = getValue(rcvString, ' ', 1).toInt();
      Serial.println("Configurado MICRO_STEP: " + String(userConfig.microStep) + " pulsos por passo");
    }
    else if(rcvString.startsWith("INICIO_RPM")){
      userConfig.inicioRPM = getValue(rcvString, ' ', 1).toInt();
      Serial.println("Configurado INICIO_RPM: " + String(userConfig.inicioRPM));
      configRampa();
    }
    else if(rcvString.startsWith("FINAL_RPM")){
      userConfig.finalRPM = getValue(rcvString, ' ', 1).toInt();
      Serial.println("Configurado FINAL_RPM: " + String(userConfig.finalRPM));
      configRampa();
    }
    else if(rcvString.startsWith("TEMPO_RAMPA")){
      userConfig.tempoRampa = getValue(rcvString, ' ', 1).toInt();
      Serial.println("Configurado TEMPO_RAMPA: " + String(userConfig.tempoRampa) + " segundos");
      configRampa();
    }
    else if(rcvString.startsWith("START") && userConfig.run == false){
      userConfig.run = true;
      runRampa();
    }
    else if(rcvString.startsWith("STOP")){
      userConfig.run = false;
    }
  }
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

*/


/*
unsigned int getRPM(unsigned long usPulse){
  unsigned long int usPulseDurationForOneRPM = getDelayPulso(1);
  unsigned long int usPulseDurationForfinalRPM = getDelayPulso(userConfig.finalRPM);
  unsigned int rpm = usPulseDurationForOneRPM / usPulse;

  Serial.println("usPulse: " + String(usPulse));
  Serial.println("usPulseDurationForOneRPM: " + String(usPulseDurationForOneRPM));
  Serial.println("rpm: " + String(rpm));
  
  static unsigned int oldRpm = 0;
  //só imprime RPM se mudar o valor
  if(oldRpm != rpm){
    Serial.println("RPM: " + String(rpm));
    oldRpm = rpm;
  }
  return rpm;
}*/
