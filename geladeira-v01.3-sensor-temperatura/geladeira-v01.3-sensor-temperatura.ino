//Programa: Teste de Display LCD 16 x 2
//Autor: FILIPEFLOP
 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 3
 
// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;
 
//Define os pinos que serão utilizados para ligação ao display
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);


//Botões de Temperatura
const int botao_up = 22;   // + PINO DIGITAL UTILIZADO PELO PUSH BUTTON - CIMA
const int botao_down = 24; // - PINO DIGITAL UTILIZADO PELO LED - BAIXO

//Temperatura Configurada pelo usuário
float temp_config = 12;

//Temperatura Sensor Geladeira
float temp_sensor_geladeira = 0;

//Controle dos Relés
//Porta ligada ao pino IN1 do modulo
int rele1_ventilador_geladeira = 50;
//Porta ligada ao pino IN2 do modulo
int rele2_resistencia_panela = 51;
//Porta ligada ao pino IN3 do modulo
int rele3_vazio = 52;
//Porta ligada ao pino IN4 do modulo
int rele4_geladeira = 53;


void setup()
{
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  lcd.clear(); //Limpa o LCD

  //Botões
  pinMode(botao_up, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO
  pinMode(botao_down, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO

  //Define pinos para o rele como saida
  pinMode(rele1_ventilador_geladeira, OUTPUT); 
  pinMode(rele2_resistencia_panela, OUTPUT);
  pinMode(rele3_vazio, OUTPUT);
  pinMode(rele4_geladeira, OUTPUT);

  //Desliga todos os reles
  digitalWrite(rele1_ventilador_geladeira, HIGH);
  digitalWrite(rele2_resistencia_panela, HIGH);
  digitalWrite(rele3_vazio, HIGH);
  digitalWrite(rele4_geladeira, HIGH);


  /* Sensor de Temperatura */
  Serial.begin(9600);
  sensors.begin();
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  if (!sensors.getAddress(sensor1, 0)) 
     Serial.println("Sensores nao encontrados !"); 
  // Mostra o endereco do sensor encontrado no barramento
  Serial.print("Endereco sensor: ");
  mostra_endereco_sensor(sensor1);
  Serial.println();
  Serial.println();

}

void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

 
void loop()
{

  /* Sensor de Temperatura */
  // Le a informacao do sensor
  sensors.requestTemperatures();
  temp_sensor_geladeira = sensors.getTempC(sensor1);


  //Botões de Temperatura
  if(digitalRead(botao_up) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      temp_config = temp_config + 1;
  }

  if(digitalRead(botao_down) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      temp_config = temp_config - 1;
  }

  // Verifica Temperatura e desliga a geladeira
  //Desliga
  if((temp_sensor_geladeira)<temp_config){ 
    digitalWrite(rele4_geladeira, HIGH);
    //Liga Ventilador
    digitalWrite(rele1_ventilador_geladeira, HIGH);
  }
  else{ 
    digitalWrite(rele4_geladeira, LOW);
    //Desliga Ventilador
    digitalWrite(rele1_ventilador_geladeira, LOW);
  }

  //Limpa a tela
  lcd.clear();

  /* Temperatura Atual */
  //Posiciona o cursor na coluna, linha;
  lcd.setCursor(0, 0);
  //Envia o texto entre aspas para o LCD
  lcd.print("G: ");
  lcd.print(temp_sensor_geladeira, 2);
  //Mostra o simbolo do grau formado pelo array
  lcd.write(223);
  
  /* Temperatura Configurada */
  lcd.setCursor(10, 0);
  lcd.print("| ");
  lcd.print(temp_config, 0);
  //Mostra o simbolo do grau formado pelo array
  lcd.write(223);

  // Mostra dados no serial monitor
  Serial.print("Temperatura Sensor Geladeira: ");
  Serial.println(temp_sensor_geladeira);

  // Mostra dados no serial monitor
  Serial.print("Temperatura Configurada Para Geladeira: ");
  Serial.println(temp_config);
  
  
  

  
  delay(200);
}
