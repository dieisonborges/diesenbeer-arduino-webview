//Programa: Teste de Display LCD 16 x 2
//Autor: FILIPEFLOP
 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Botões de Temperatura
const int botao_up = 6;   // + PINO DIGITAL UTILIZADO PELO PUSH BUTTON - CIMA
const int botao_down = 7; // - PINO DIGITAL UTILIZADO PELO LED - BAIXO

//Temperatura Configurada pelo usuário
int temp_config = 12;

//Temperatura Sensor Geladeira
int temp_sensor_geladeira = 12;

//Controle dos Relés
//Porta ligada ao pino IN1 do modulo
int rele1_geladeira = 50;
//Porta ligada ao pino IN2 do modulo
int rele2 = 51;
//Porta ligada ao pino IN3 do modulo
int rele3 = 52;
//Porta ligada ao pino IN4 do modulo
int rele4 = 53;

//Array simbolo grau
byte grau[8] ={ B00001100,
                B00010010,
                B00010010,
                B00001100,
                B00000000,
                B00000000,
                B00000000,
                B00000000,};
 
void setup()
{
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  lcd.clear(); //Limpa o LCD
  lcd.createChar(0, grau);

  //Botões
  pinMode(botao_up, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO
  pinMode(botao_down, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO

  //Define pinos para o rele como saida
  pinMode(rele1_geladeira, OUTPUT); 
  pinMode(rele2, OUTPUT);
  pinMode(rele3, OUTPUT);
  pinMode(rele4, OUTPUT);

  //Desliga todos os reles
  digitalWrite(rele1_geladeira, HIGH);
  digitalWrite(rele2, HIGH);
  digitalWrite(rele3, HIGH);
  digitalWrite(rele4, HIGH);

}
 
void loop()
{
  

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
    digitalWrite(rele1_geladeira, HIGH);
  }
  else{ 
    digitalWrite(rele1_geladeira, LOW);
  }

  //Limpa a tela
  lcd.clear();

  /* Temperatura Atual */
  //Posiciona o cursor na coluna, linha;
  lcd.setCursor(0, 0);
  //Envia o texto entre aspas para o LCD
  lcd.print("Temp: ");
  lcd.print(temp_sensor_geladeira);
  //Mostra o simbolo do grau formado pelo array
  lcd.write((byte)0);
  
  /* Temperatura Configurada */
  //Posiciona o cursor na coluna, linha;
  lcd.setCursor(0, 1);
  lcd.print("Conf: ");
  lcd.print(temp_config);
  //Mostra o simbolo do grau formado pelo array
  lcd.write((byte)0);
  

  
  delay(300);
}
