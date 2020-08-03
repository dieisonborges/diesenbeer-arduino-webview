//Programa: Teste de Display LCD 16 x 2
//Autor: FILIPEFLOP
 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

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
}
 
void loop()
{
  //Limpa a tela
  lcd.clear();

  /* Temperatura Atual */
  //Posiciona o cursor na coluna, linha;
  lcd.setCursor(0, 0);
  //Envia o texto entre aspas para o LCD
  lcd.print("Temp: ");
  lcd.print("12");
  //Mostra o simbolo do grau formado pelo array
  //lcd.write((byte)0);
  
  /* Temperatura Configurada */
  //Posiciona o cursor na coluna, linha;
  lcd.setCursor(0, 1);
  lcd.print("Conf: ");
  lcd.print("18");
  //Mostra o simbolo do grau formado pelo array
  lcd.write((byte)0);
  delay(5000);
}
