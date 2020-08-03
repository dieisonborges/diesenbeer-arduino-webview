//Programa: Cervejaria Artesanal
//Autor: DIEISON
 
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>
 
// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 2

 
// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

/* --------- ETHERNET ----------- */
//Informacoes de endereco IP, gateway, mascara de rede
byte mac[] = { 0xA4, 0x28, 0x72, 0xCA, 0x55, 0x2F };
byte ip[] = { 192, 168, 100, 4 };
byte gateway[] = { 192, 168, 100, 1 };
byte subnet[] = { 255, 255, 255, 0 };
EthernetServer server(80);

/* Variaveis de Controle via GET */
String readString;
boolean geladeira_ligada = false;
boolean bomba_ligada = false;
boolean panela_ligada = false;


/* Temporizador ------------------- */
unsigned long tempo;
unsigned long tempo_play = 0;
unsigned long tempo_segundo = 0;
unsigned long tempo_minuto = 0;
unsigned long tempo_minuto_corrido = 0;
unsigned long tempo_hora = 0;
int controle_temporizador = 0; //1 = PLAY 0 = STOP
/* FIM Temporizador ------------------- */

int deviceCount = 0;
float tempC;

//Alterna Display
int alterna_display = 0;

// Addresses of 3 DS18B20s
uint8_t sensor1[8] = { 0x28, 0xFF, 0x66, 0x0D, 0xC3, 0x16, 0x03, 0x4B };
uint8_t sensor2[8] = { 0x28, 0xFF, 0xEE, 0xEE, 0xC2, 0x16, 0x03, 0xD4 };
 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(11, 12, 7, 6, 4, 3);

//Botões de Temperatura
const int botao_up = 22;   // + PINO DIGITAL UTILIZADO PELO PUSH BUTTON - CIMA
const int botao_down = 24; // - PINO DIGITAL UTILIZADO PELO LED - BAIXO

//Botão Reset LCD
const int botao_reset_lcd = 26;

//Controle dos Relés
//Porta ligada ao pino IN1 do modulo
//int rele1_ventilador_geladeira = 50;
//Porta ligada ao pino IN2 do modulo
//int rele2_vazio = 51;
//Porta ligada ao pino IN3 do modulo
//int rele3_vazio = 52;
//Porta ligada ao pino IN4 do modulo
int rele4_geladeira = 53;

//Resistência e Bomba
int rele1_bomba = 49;   // Bomba de Circulação Panela - IN1
int rele2_resistencia_panela = 48; // Resistência da Panela - IN2

//Status LCD (ON - 1 ou OFF - 0)
int lcd_status = 1;

//Temperatura Configurada pelo usuário
float temp_config_geladeira = 2; // 2°

//Temperatura Sensor Geladeira
float temp_sensor_geladeira = 0; // 0°

//Temperatura Configurada pelo usuário
float temp_config_panela = 65; // 65º

//Temperatura Sensor Geladeira
float temp_sensor_panela = 0; // 65º

//Contador Delay Temperatura
int temp_cont_set = 30;
int temp_cont = 0;

void setup()
{
  //Inicia LCD
  reset_lcd();

  Serial.println("DiensenBEER - Cervejaria Artesanal");

  //Inicializa Ethernet Shield
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();

  //Botão Reset LCD
  pinMode(botao_reset_lcd, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO

  //Botões
  pinMode(botao_up, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO
  pinMode(botao_down, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO

  //Define pinos para o rele como saida
  //pinMode(rele1_ventilador_geladeira, OUTPUT); 
  //pinMode(rele2_vazio, OUTPUT);
  //pinMode(rele3_vazio, OUTPUT);
  pinMode(rele4_geladeira, OUTPUT);
  pinMode(rele1_bomba, OUTPUT);
  pinMode(rele2_resistencia_panela, OUTPUT);

  //Liga todos os reles
  //digitalWrite(rele1_ventilador_geladeira, LOW);
  //digitalWrite(rele2_vazio, LOW);
  //digitalWrite(rele3_vazio, LOW);
  digitalWrite(rele4_geladeira, HIGH);
  //Desliga todos os reles
  digitalWrite(rele1_bomba, HIGH);
  digitalWrite(rele2_resistencia_panela, HIGH);


  /* Sensor de Temperatura */
  sensors.begin();  // Start up the library
  Serial.begin(9600);
  
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");
  Serial.println();

}


//Reinicializa o LCD
void reset_lcd(){ 
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  //lcd.clear(); //Limpa o LCD
}


void loop()
{
  
  
  /* Sensor de Temperatura */
  // Le a informacao do sensor
  sensors.requestTemperatures();
  temp_sensor_geladeira = sensors.getTempC(sensor1);
  temp_sensor_panela = sensors.getTempC(sensor2);

  //Botão Reset Display
    //Botão Reset Display TB é utilizado para alternar o display

  if(digitalRead(botao_reset_lcd) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      reset_lcd();
      //Se = 0 display panela se = 1 display da ressitencia
      alterna_display++;
      if(alterna_display>3){
        alterna_display = 0;
      }
  }

  //LIGA / DESLIGA LCD
  /*
  if((digitalRead(botao_up) == LOW)and(digitalRead(botao_down) == LOW)){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      
      if(lcd_status == 1){
        lcd_status = 0;
        lcd.noDisplay();
      }else{
        lcd_status = 1;
        lcd.display();
      }
  }
  */ 

  //Contador Delay Temperatura
  temp_cont = temp_cont -1;

  if(temp_cont<=0){
    /* ----------------------------------------------- GELADEIRA -------------------------------- */ 
    
    // Verifica Temperatura e desliga a geladeira
    //Desliga
    if((temp_sensor_geladeira)<temp_config_geladeira){ 
      digitalWrite(rele4_geladeira, HIGH);
      geladeira_ligada = false;
    }
    else{ 
      digitalWrite(rele4_geladeira, LOW);
      geladeira_ligada = true;
    }

    /* ----------------------------------------------- PANELA -------------------------------- */    
    
    // Verifica Temperatura e desliga a resistencia da panela
    //Desliga
    if((temp_sensor_panela)>temp_config_panela){ 
      digitalWrite(rele2_resistencia_panela, HIGH); //PIN 48      
      panela_ligada = false;
    }
    else{ 
      digitalWrite(rele2_resistencia_panela, LOW); //PIN 48 
      panela_ligada = true;
    }

    //Bomba de Circulação
    //if(bomba_ligada==true){ 
      //digitalWrite(rele1_bomba, LOW); //PIN 49      
      //bomba_ligada = true;
    //}
    //else{ 
      //digitalWrite(rele1_bomba, HIGH); //PIN 49
      //bomba_ligada = false;
    //}
    

    /* ------------------------------------------------------------------------------------------ */

    //Reseta Delay de Temperatura
    temp_cont = temp_cont_set;
    
  }

  

  /* *********************************************** LCD **************************************** */
  if(alterna_display == 0){
    //Display da Geladeira
    //Limpa a tela
    lcd.clear();

    /* ----------------------------------- GELADEIRA ----------------------- */
    //Botões de Temperatura
    if(digitalRead(botao_up) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
        temp_config_geladeira = temp_config_geladeira + 1;
    }
  
    if(digitalRead(botao_down) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
        temp_config_geladeira = temp_config_geladeira - 1;
    }
    /* Temperatura Atual */
    //Posiciona o cursor na coluna, linha;
    lcd.setCursor(0, 0);
    //Envia o texto entre aspas para o LCD
    lcd.print("Gelad.: ");  
    lcd.print(temp_sensor_geladeira, 2);
    lcd.write(223); 
    /* Temperatura Configurada */
    lcd.setCursor(0, 1);
    lcd.print("Conf: "); 
    lcd.print(temp_config_geladeira, 0);
    //Mostra o simbolo do grau formado pelo array
    lcd.write(223);    
    /* ----------------------------------------------------------------- */ 
  }
  if(alterna_display == 1){
    //Display da Panela
    //Limpa a tela
    lcd.clear();
    
    /* ----------------------------------- PANELA ----------------------- */
    //Botões de Temperatura
    if(digitalRead(botao_up) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
        temp_config_panela = temp_config_panela + 1;
    }
  
    if(digitalRead(botao_down) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
        temp_config_panela = temp_config_panela - 1;
    }
    /* Temperatura Atual */
    //Posiciona o cursor na coluna, linha;
    lcd.setCursor(0, 0);
    //Envia o texto entre aspas para o LCD
    lcd.print("Panela: ");  
    lcd.print(temp_sensor_panela, 2);
    lcd.write(223);
    /* Temperatura Configurada */
    lcd.setCursor(0, 1);
    lcd.print("Conf: "); 
    lcd.print(temp_config_panela, 0);
    lcd.write(223);    
    
  }
  if(alterna_display == 2){
    //Display da Panela Fervura
    //Limpa a tela
    lcd.clear();
    
    /* ----------------------------------- PANELA ----------------------- */
    //Botões de Temperatura
    if(digitalRead(botao_up) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
        temp_config_panela = 100;
    }
  
    if(digitalRead(botao_down) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
        temp_config_panela = 60;
    }
    /* Temperatura Atual */
    //Posiciona o cursor na coluna, linha;
    lcd.setCursor(0, 0);
    //Envia o texto entre aspas para o LCD
    lcd.print("Ferv.: ");  
    lcd.print(temp_sensor_panela, 2);
    lcd.write(223);
    /* Temperatura Configurada */
    lcd.setCursor(0, 1);
    lcd.print("Conf: "); 
    lcd.print(temp_config_panela, 0);
    lcd.write(223);   
    
  }

  
  if(alterna_display == 3){
    //Temporizador
    //Limpa a tela
    lcd.clear();    
    /* -------------------------TEMPORIZADOR----------------------------------------------------------------- */
    if(digitalRead(botao_up) == LOW)
    {
      tempo = millis();
      tempo_segundo=0;
      tempo_minuto=0;
      tempo_hora=0;
      tempo_minuto_corrido=0;
      controle_temporizador = 1;
    }
  
    if(digitalRead(botao_down) == LOW)
    {    
      controle_temporizador = 0;
    }
  
    if(controle_temporizador==1){
      lcd.clear();
      /* Temperatura Atual */
      //Posiciona o cursor na coluna, linha;
      lcd.setCursor(0, 0);
      //Envia o texto entre aspas para o LCD
      lcd.print("Crono: ");  
      lcd.print(temp_sensor_panela, 2);
      lcd.write(223);
      
      tempo_play = millis() - tempo;
      tempo_segundo = tempo_play / 1000;
      
      lcd.setCursor(0, 1);    
      if(tempo_segundo>=60){
        tempo = millis();
        tempo_minuto++;
        tempo_minuto_corrido++;
      }
      if(tempo_minuto>=60){
        tempo = millis();
        tempo_minuto=0;
        tempo_hora++;
      }
    
      lcd.print(tempo_hora);
      lcd.print(":");
      lcd.print(tempo_minuto);
      lcd.print(":");
      lcd.print(tempo_segundo); 
      lcd.print(" (");
      lcd.print(tempo_minuto_corrido); 
      lcd.print("m)");
      
    }
    if(controle_temporizador==0){
      lcd.clear();
      //Posiciona o cursor na coluna, linha;
      lcd.setCursor(0, 0);
      //Envia o texto entre aspas para o LCD
      lcd.print("Crono: ");  
      lcd.print(temp_sensor_panela, 2);
      lcd.write(223);
      
      lcd.setCursor(0, 1); 
      lcd.print(tempo_minuto_corrido);
      lcd.print(" mins ");
      lcd.print(tempo_segundo);
      lcd.print(" s");
    }
   
    /* -------------------------FIM TEMPORIZADOR------------------------------------------------------------- */
   
    
  }

  /* ---------- Contador Display ------------*/
  /* Contador */
    lcd.setCursor(15, 1);
    if(temp_cont==0){
      lcd.print("+");      
    }else{        
      if((temp_cont%2)==0){
        lcd.print("-");
      }else{
        lcd.print("|");
      }
    }
  /* ---------- FIM Contador Display ------------*/


  //Serial.println("============================================");
  
  /* ------------------ GELADEIRA ---------------------- */
  // Mostra dados no serial monitor

  /*
  Serial.print("Temp. Geladeira: ");
  Serial.print(temp_sensor_geladeira);
  Serial.print("º");
  Serial.print(" | Conf: ");
  Serial.print(temp_config_geladeira);
  Serial.print("º");
  Serial.println("");
  Serial.println("");
  */

  /* ------------------ PANELA ---------------------- */

  // Mostra dados no serial monitor
  /*
  Serial.print("Temp. Panela: ");
  Serial.print(temp_sensor_panela);
  Serial.print("º");
  Serial.print(" | Conf: ");
  Serial.print(temp_config_panela);
  Serial.print("º");
  Serial.println("");
  Serial.println("");  
  
  Serial.println("");
  */

  /* ----------------------------------------- ETHERNET -----------------------------------*/
  //Ethernet
  EthernetClient client = server.available();
  if (client) {
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == 'n')
        {
          //Controle do rele1
          Serial.println(readString);

          //Aumenta temperatura da geladeira
          if (readString.indexOf("?g_up") > 0)
          {
            temp_config_geladeira++;
            Serial.println("Aumenta Temperatura Geladeira");
          }

          //Diminui temperatura da geladeira
          if (readString.indexOf("?g_down") > 0)
          {
            temp_config_geladeira--;
            Serial.println("Diminui Temperatura Geladeira");
          }

          //Geladeira controlada por termostato -270º
          if (readString.indexOf("?g_ter") > 0)
          {
            temp_config_geladeira=-270;
            Serial.println("Geladeira controlada por termostato -270º");
          }

          //Geladeira desligada 100º
          if (readString.indexOf("?g_des") > 0)
          {
            temp_config_geladeira=100;
            Serial.println("Geladeira desligada 100º");
          }

          //Configura geladeira para 2º
          if (readString.indexOf("?g_2") > 0)
          {
            temp_config_geladeira=2;
            Serial.println("Configura geladeira para 2º");
          }
          
          //Aumenta temperatura da panela
          if (readString.indexOf("?p_up") > 0)
          {
            temp_config_panela++;
            Serial.println("Aumenta Temperatura Panela");
          }

          //Diminui temperatura da panela
          if (readString.indexOf("?p_down") > 0)
          {
            temp_config_panela--;
            Serial.println("Diminui Temperatura Panela");
          }

          //Configura panela para 60º
          if (readString.indexOf("?p_60") > 0)
          {
            temp_config_panela=60;
            Serial.println("Configura panela para 60º");
          }

          //Desliga resistencia da panela -270º
          if (readString.indexOf("?p_des") > 0)
          {
            temp_config_panela=-270;
            Serial.println("Desliga Resistência Panela -270º");
          }

          //Liga fervura panela 100º
          if (readString.indexOf("?p_fer") > 0)
          {
            temp_config_panela=100;
            Serial.println("Liga fervura panela 100º");
          }

          //Liga bomba de circulação
          if (readString.indexOf("?b_on") > 0)
          {
            bomba_ligada = true;
            digitalWrite(rele1_bomba, LOW); //PIN 49 
            Serial.println("Liga bomba de circulação");
          }

          //Desliga bomba de circulação
          if (readString.indexOf("?b_off") > 0)
          {
            bomba_ligada = false;
            digitalWrite(rele1_bomba, HIGH); //PIN 49
            Serial.println("Desliga bomba de circulação");
          }    

          //Temporizador Play
          if (readString.indexOf("?t_play") > 0)
          {
            tempo = millis();
            tempo_segundo=0;
            tempo_minuto=0;
            tempo_hora=0;
            tempo_minuto_corrido=0;
            controle_temporizador = 1;
            Serial.println("Temporizador Play");
          }

          //Temporizador Stop
          if (readString.indexOf("?t_stop") > 0)
          {    
            controle_temporizador = 0;
            Serial.println("Temporizador Stop");
          }
          
          readString = "";      
        
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.print("<!DOCTYPE html> <html> <head> <meta charset='utf-8'> <meta http-equiv='X-UA-Compatible' content='IE=edge'>");
          client.print("<title>Diensen Beer</title> <meta name='viewport' content='width=device-width, initial-scale=1'> ");
          client.print("<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.14.0/css/all.css'> ");
          client.print("<link rel='stylesheet' href='https://adminlte.io/themes/v3/dist/css/adminlte.min.css'> ");
          client.print("<link href='https://fonts.googleapis.com/css?family=Source+Sans+Pro:300,400,400i,700' rel='stylesheet'> ");
          client.print("<link rel='stylesheet' href='https://code.ionicframework.com/ionicons/2.0.1/css/ionicons.min.css'> ");
          client.print("<link rel='shortcut icon' href='https://i.pinimg.com/originals/79/93/06/7993060d35cc9e039a412f1f60abefa7.png' type='image/x-icon'> ");
          client.print("<link rel='icon' href='https://i.pinimg.com/originals/79/93/06/7993060d35cc9e039a412f1f60abefa7.png' type='image/x-icon'> ");
          client.print("</head> <body class='hold-transition sidebar-mini sidebar-collapse'> ");
          client.print("<div class='wrapper'>");
          client.print("<nav class='main-header navbar navbar-expand navbar-white navbar-light'>");
          client.print("<ul class='navbar-nav'>");
          client.print("<li class='nav-item'>");
          client.print("<a class='nav-link' data-widget='pushmenu' href='#' role='button'><i class='fas fa-bars'></i></a>");
          client.print("</li>");          
          client.print("</ul>");
          client.print("</nav> ");
          client.print("<aside class='main-sidebar sidebar-dark-primary elevation-4'> ");
          client.print("<a href='/' class='brand-link'>");
          client.print("<img src='https://i.pinimg.com/originals/79/93/06/7993060d35cc9e039a412f1f60abefa7.png' ");
          client.print("alt='Diensen Beer' ");
          client.print("class='brand-image img-circle elevation-3'");
          client.print("style='opacity: .8'> ");
          client.print("<span class='brand-text font-weight-light'>Diensen Beer</span> ");
          client.print("</a> ");
          client.print("<div class='sidebar'>");   
          client.print("<nav class='mt-2'> ");
          client.print("<ul class='nav nav-pills nav-sidebar flex-column' data-widget='treeview'");
          client.print("role='menu' data-accordion='false'> ");
          client.print("<li class='nav-item has-treeview'>");
          client.print("<a href='#' class='nav-link'>");
          client.print("<i class='nav-icon fas fa-beer'></i>");
          client.print("<p> ");
          client.print("HomeBrew");
          client.print("</p>");
          client.print("</a>");      
          client.print("</li> ");
          client.print("</ul>");
          client.print("</nav> ");
          client.print("</div> ");
          client.print("</aside> ");
          client.print("<div class='content-wrapper'> ");
          client.print("<section class='content-header'>");
          client.print("<div class='container-fluid'>");
          client.print("<div class='row mb-2'>");
          client.print("<div class='col-sm-6'>");
          client.print("<h1> <i class='fas fa-beer'></i>  Equipamentos <small><a href='/'><i class='fas fa-home'></i> HomeBrew</a></small></h1> ");
          client.print("</div>");
          client.print("</div>");
          client.print("</div> ");
          client.print("</section>");
          client.print("<section class='content'> ");    
          client.print("<div class='row'>");
  
          
          client.print("<div class='col-md-3'> ");
          client.print("<div class='small-box bg-gradient-info'> ");
          client.print("<div class='inner'>");
          client.print("<h3> ");                                                                                                                                
          client.print(temp_sensor_geladeira);
          client.print("º <small>");                                                                                                                        //
          client.print(temp_config_geladeira);
          client.print("º</small>");
          client.print("</h3>");
          client.print("<p>Geladeira");
          if(geladeira_ligada==true){
            client.print("<small class='float-right'><i class='fas fa-snowman'></i> ligada </small>");
          }
          client.print("</p>");
          client.print("</div>");
          client.print("<div class='icon'> ");
          client.print("<i class='ion ion-thermometer'></i>");
          client.print("</div>");
          client.print("<div class='card-footer col-md-12'>");
          client.print("<a href='?g_up' class='btn btn-primary float-left'><i class='fas fa-plus-square'></i></a>");
          client.print("<a href='?g_down' class='btn btn-primary ml-2'><i class='fas fa-minus-square'></i></a>");
          client.print("<a href='.?g_2=' class='btn btn-default float-right'>2º</a>");
          client.print("</div>");
          client.print("<a href='?g_des' class='small-box-footer'>Desligar Geladeira <i class='fas fa-power-off'></i></a>");
          client.print("<a href='?g_ter' class='small-box-footer'>Usar Termostato <i class='fas fa-temperature-low'></i></a>");
          client.print("</div></div>");
  
         
          client.print("<div class='col-md-3'> ");
          client.print("<div class='small-box bg-gradient-warning'>");
          client.print("<div class='inner'>");
          client.print("<h3>");                                                                                                                           
          client.print(temp_sensor_panela);
          client.print("º <small>");                                                                                                                           //
          client.print(temp_config_panela);
          client.print("º</small> ");
          client.print("</h3>");
          client.print("<p>Panela");
          if(panela_ligada==true){
            client.print("<small class='text-danger float-right'><i class='fas fa-bolt'></i> ligada </small>");
          }
          client.print("</p>");
          client.print("</div>");
          client.print("<div class='icon'> ");
          client.print("<i class='ion ion-thermometer'></i>");
          client.print("</div>");
          client.print("<div class='card-footer col-md-12'>");
          client.print("<a href='.?p_up' class='btn btn-default float-left'><i class='fas fa-plus-square'></i></a>");
          client.print("<a href='.?p_down' class='btn btn-default ml-2'><i class='fas fa-minus-square'></i></a>");
          client.print("<a href='.?p_60' class='btn btn-default float-right'>60º</a>");
          client.print("</div>");
          client.print("<a href='?p_des' class='small-box-footer'>Desligar Resistência <i class='fas fa-power-off'></i></a>");
          client.print("<a href='?p_fer' class='small-box-footer'>Ferver Panela <i class='fas fa-fire text-red'></i></a>");
          client.print("</div></div>");    
  
  
          client.print("<div class='col-md-3'> ");
          client.print("<div class='small-box bg-secondary'>");
          client.print("<div class='inner'>");
          client.print("<h3>");                                                                                                                           
          client.print(tempo_hora);
          client.print(":");
          client.print(tempo_minuto);
          client.print(":");
          client.print(tempo_segundo);         
          client.print("<small>");                                                                                                                           //
          client.print(" (");
          client.print(tempo_minuto_corrido); 
          client.print("m)");
          client.print("</small> ");
          client.print("</h3>");        
          client.print("<p>Temporizador</p>");
          client.print("</div>");
          client.print("<div class='icon'> ");
          client.print("<i class='ion ion-clock'></i>");
          client.print("</div>");
          client.print("<div class='card-footer col-md-12'>");
          client.print("<a href='?t_play' class='btn btn-default float-left'><i class='fas fa-stopwatch'></i></a>");
          client.print("<a href='?t_stop' class='btn btn-default float-right'><i class='far fa-stop-circle'></i></a>");
          client.print("</div>");
          client.print("<a href='?t_play' class='small-box-footer'>Iniciar/Reiniciar <i class='fas fa-stopwatch'></i></a>");
          client.print("<a href='?t_stop' class='small-box-footer'>Parar <i class='far fa-stop-circle'></i></a>");
          client.print("</div></div>");   
  
  
          client.print("<div class='col-md-3'> ");
          client.print("<div class='small-box bg-default'>");
          client.print("<div class='inner'>");
          client.print("<h3>Bomba</h3>");
          client.print("<p>Recirculação");
          if(bomba_ligada==true){
            client.print("<small class='text-danger float-right'><i class='fas fa-shower'></i> ligada </small>");
          }
          client.print("</p>");
          client.print("</div>");
          client.print("<div class='icon'> ");
          client.print("<i class='ion ion-refresh'></i>");
          client.print("</div>");
          client.print("<div class='card-footer col-md-12'>");
          client.print("<a href='?b_on' class='btn btn-default float-left'><i class='fas fa-play-circle'></i></a>");
          client.print("<a href='?b_off' class='btn btn-default float-right'><i class='far fa-stop-circle'></i></a>");
          client.print("</div>");
          client.print("<a href='?b_on' class='small-box-footer text-dark'>Ligar Bomba <i class='fas fa-play-circle'></i></a>");
          client.print("<a href='?b_off' class='small-box-footer text-dark'>Desligar Bomba <i class='far fa-stop-circle'></i></a>");
          client.print("</div></div>");   
  
           
          client.print("</div></div></div></div>");
          
  
          
          client.print("<script src='https://adminlte.io/themes/v3/plugins/jquery/jquery.min.js'></script>");
          client.print("<script src='https://adminlte.io/themes/v3/plugins/bootstrap/js/bootstrap.bundle.min.js'></script>");
          client.print("<script src='https://adminlte.io/themes/v3/dist/js/adminlte.min.js'></script>");
          client.print("<script src='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.14.0/js/all.js'></script>");
          client.print("</body>");
          client.print("</html>"); 
  
          
  
          
          client.stop();
        }
        
      }
    }
  }
  /* ------------------------------ FIM ETHERNET --------------------------------------- */
  
  

  
  delay(150);
}
