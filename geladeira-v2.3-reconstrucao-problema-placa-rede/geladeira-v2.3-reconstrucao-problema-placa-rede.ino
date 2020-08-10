#include <SPI.h> //INCLUSÃO DE BIBLIOTECA
#include <UIPEthernet.h> //INCLUSÃO DE BIBLIOTECA
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

// ************* Sensor de Temperatura ************
// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 23
// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Addresses of 3 DS18B20s
uint8_t sensor1[8] = { 0x28, 0xFF, 0x66, 0x0D, 0xC3, 0x16, 0x03, 0x4B };
uint8_t sensor2[8] = { 0x28, 0xFF, 0xEE, 0xEE, 0xC2, 0x16, 0x03, 0xD4 };

//Temperatura Configurada pelo usuário
float temp_config_geladeira = 2; // 2°

//Temperatura Sensor Geladeira
float temp_sensor_geladeira = 0; // 0°

//Temperatura Configurada pelo usuário
float temp_config_panela = 65; // 65º

//Temperatura Sensor Geladeira
float temp_sensor_panela = 0; // 65º

// ************* ETHERNET *******************
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //ATRIBUIÇÃO DE ENDEREÇO MAC AO ENC28J60
byte ip[] = { 192, 168, 100, 4 }; //COLOQUE UMA FAIXA DE IP DISPONÍVEL DO SEU ROTEADOR. EX: 192.168.1.110  **** ISSO VARIA, NO MEU CASO É: 192.168.0.175
EthernetServer server(80); //PORTA EM QUE A CONEXÃO SERÁ FEITA


// ************* Relés *******************
//Porta ligada ao pino IN1 do modulo 1
int rele1_m1_vazio = 31;
//Porta ligada ao pino IN2 do modulo 1
int rele2_m1_vazio = 33;
//Porta ligada ao pino IN3 do modulo 1
int rele3_m1_vazio = 35;
//Porta ligada ao pino IN4 do modulo 1
int rele4_m1_geladeira = 37; //Controle Temperatura Geladeira

//Resistência e Bomba
//Porta ligada ao pino IN1 do modulo 2
int rele1_m2_bomba = 48;   // Bomba de Circulação Panela
//Porta ligada ao pino IN1 do modulo 2
int rele2_m2_resistencia_panela = 46; // Resistência da Panela

//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(49, 47, 45, 43, 41, 39);

String readString = String(30); //VARIÁVEL PARA BUSCAR DADOS NO ENDEREÇO (URL)

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

//Botões de Temperatura
const int botao_up = 25;   // + PINO DIGITAL UTILIZADO PELO PUSH BUTTON - CIMA
const int botao_down = 27; // - PINO DIGITAL UTILIZADO PELO LED - BAIXO

//Botão Reset LCD
const int botao_reset_lcd = 29;

int timer_bug_cont = 0;

void setup() {
  Ethernet.begin(mac, ip); //PASSA OS PARÂMETROS PARA A FUNÇÃO QUE VAI FAZER A CONEXÃO COM A REDE
  server.begin(); //INICIA O SERVIDOR PARA RECEBER DADOS NA PORTA 80

  pinMode(rele1_m1_vazio, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(rele1_m1_vazio, HIGH); //LED INICIA DESLIGADO
  pinMode(rele2_m1_vazio, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(rele2_m1_vazio, HIGH); //LED INICIA DESLIGADO
  pinMode(rele3_m1_vazio, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(rele3_m1_vazio, HIGH); //LED INICIA DESLIGADO
  pinMode(rele4_m1_geladeira, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(rele4_m1_geladeira, HIGH); //LED INICIA DESLIGADO
  pinMode(rele1_m2_bomba, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(rele1_m2_bomba, HIGH); //LED INICIA DESLIGADO
  pinMode(rele2_m2_resistencia_panela, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(rele2_m2_resistencia_panela, HIGH); //LED INICIA DESLIGADO

  //Inicia LCD
  reset_lcd();

  //Botão Reset LCD
  pinMode(botao_reset_lcd, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO

  //Botões
  pinMode(botao_up, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO
  pinMode(botao_down, INPUT_PULLUP); //DEFINE O PINO COMO ENTRADA / "_PULLUP" É PARA ATIVAR O RESISTOR INTERNO

}

//Reinicializa o LCD
void reset_lcd(){ 
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  //lcd.clear(); //Limpa o LCD
}

void loop() {

  //Controle para não delisgar e ligar rápido
  timer_bug_cont++;
  if(timer_bug_cont==100){
    timer_bug_cont=0;
  }
  
  /* Sensor de Temperatura */
  sensors.requestTemperatures();
  temp_sensor_geladeira = sensors.getTempC(sensor1);
  temp_sensor_panela = sensors.getTempC(sensor2);


  /* ---------------------- BOTOES -------------------- */
  if(digitalRead(botao_reset_lcd) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      reset_lcd();      
  }
  
  if(digitalRead(botao_up) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      temp_config_geladeira++;
  }

  if(digitalRead(botao_down) == LOW){ //SE A LEITURA DO PINO FOR IGUAL A LOW, FAZ
      temp_config_geladeira--;
  }
  /* ---------------------- FIM BOTOES -------------------- */

  /* ------------------ LCD - Temperatura Atual ------------------- */
  //Posiciona o cursor na coluna, linha;
  lcd.setCursor(0, 0);
  //Envia o texto entre aspas para o LCD
  lcd.print("G: ");  
  lcd.print(temp_sensor_geladeira, 2);
  lcd.write(223); 
  lcd.print("|"); 
  lcd.print(temp_config_geladeira, 2);
  lcd.write(223); 
  /* Temperatura Configurada */
  lcd.setCursor(0, 1);
  lcd.print("P: "); 
  lcd.print(temp_sensor_panela, 2);
  lcd.write(223); 
  lcd.print("|"); 
  lcd.print(temp_config_panela, 2);
  //Mostra o simbolo do grau formado pelo array
  lcd.write(223); 
  /* ------------------ FIM LCD - Temperatura Atual ------------------- */

  //Controle da Resistência Brassagem
  if(temp_config_panela<temp_sensor_panela){
    if(timer_bug_cont==0){
      digitalWrite(rele2_m2_resistencia_panela, HIGH); //DESLIGA PARA ESFRIAR
      panela_ligada = false;
    }
  }else if(temp_config_panela>temp_sensor_panela){
    if(timer_bug_cont==0){
      digitalWrite(rele2_m2_resistencia_panela, LOW); //LIGA PARA ESQUENTAR
      panela_ligada = true;
    }
  }

  //Controle da Geladeira
  if(temp_config_geladeira>temp_sensor_geladeira){
    if(timer_bug_cont==0){
      digitalWrite(rele4_m1_geladeira, HIGH); //DESLIGA PARA ESQUENTAR
      geladeira_ligada = false;
    }
  }else if(temp_config_geladeira<temp_sensor_geladeira){
    if(timer_bug_cont==0){
      digitalWrite(rele4_m1_geladeira, LOW); //LIGA PARA GELAR
      geladeira_ligada = true;
    }
  }
  
  EthernetClient client = server.available(); //CRIA UMA CONEXÃO COM O CLIENTE
  if (client) { // SE EXISTE CLIENTE FAZ
    while (client.connected()) {//ENQUANTO EXISTIR CLIENTE CONECTADO, FAZ
      if (client.available()) { //SE O CLIENTE ESTÁ HABILITADO, FAZ
        char c = client.read(); //LÊ CARACTER A CARACTER DA REQUISIÇÃO HTTP
        if (readString.length() < 100) //SE O ARRAY FOR MENOR QUE 100, FAZ
        {
          readString += c; // "readstring" VAI RECEBER OS CARACTERES LIDO
        }
        if (c == '\n')
        {
          //SE ENCONTRAR "\n" É O FINAL DO CABEÇALHO DA REQUISIÇÃO HTTP, FAZ
          if (readString.indexOf("?") < 0)
          {
            //SE ENCONTRAR O CARACTER "?", FAZ
          }


          /* -------------------------TEMPORIZADOR----------------------------------------------------------------- */
          if (readString.indexOf("temporizador=restart") > 0)
          {
            tempo = millis();
            tempo_segundo=0;
            tempo_minuto=0;
            tempo_hora=0;
            tempo_minuto_corrido=0;
            controle_temporizador = 1;
          }
          else if (readString.indexOf("temporizador=pause") > 0) {
            controle_temporizador = 0;
          }
          else if (readString.indexOf("temporizador=play") > 0) {
            controle_temporizador = 1;
          }          
        
          if(controle_temporizador==1){      
            
            tempo_play = millis() - tempo;
            tempo_segundo = tempo_play / 1000;            
  
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
            
          } 
         
          /* -------------------------FIM TEMPORIZADOR------------------------------------------------------------- */
          
          

          
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

          

          /* -------------------------- BOMBA DE RECIRCULAÇÃO --------------------------------------------*/
          if (readString.indexOf("bomba=true") > 0)
          {
            //SE ENCONTRAR O PARÂMETRO "bomba=true", FAZ
            digitalWrite(rele1_m2_bomba, LOW); //LIGA
            bomba_ligada = true; //VARIÁVEL RECEBE VALOR TRUE(SIGNIFICA QUE ESTÁ LIGADO)
          }
          else if (readString.indexOf("bomba=false") > 0) {
            //SE ENCONTRAR O PARÂMETRO "bomba=false", FAZ
            digitalWrite(rele1_m2_bomba, HIGH); //DESLIGA
            bomba_ligada = false; //VARIÁVEL RECEBE VALOR FALSE(SIGNIFICA QUE ESTÁ DESLIGADO)
          }
          
          client.print("<li class='nav-item has-treeview'>");
          client.print("<a href='#' class='nav-link'>");
          client.print("<i class='nav-icon fas fa-shower'></i></i>");
          client.print("<p>");
          client.print("Bomba Circulação");
          client.print("<i class='right fas fa-angle-left'></i>");
          client.print("</p>");
          client.print("</a>");
          client.print("<ul class='nav nav-treeview' style='display: none;'>");
          client.print("<li class='nav-item'>");
          client.print("<a href='.?bomba=true' class='nav-link'>");
          client.print("<i class='far fa-circle nav-icon text-danger'></i>");
          client.print("<p>Ligar</p>");
          client.print("</a>");
          client.print("</li>");
          
      
          client.print("<li class='nav-item'>");
          client.print("<a href='.?bomba=false' class='nav-link'>");
          client.print("<i class='far fa-circle nav-icon text-success'></i>");
          client.print("<p>Desligar</p>");
          client.print("</a>");               
          client.print("</li>");             
          client.print("</ul>");
          client.print("</li>");
          /* -------------------------- FIM BOMBA DE RECIRCULAÇÃO --------------------------------------------*/

          
          
          client.print("</ul>");
          client.print("</nav> ");
          client.print("</div> ");
          client.print("</aside> ");
          client.print("<div class='content-wrapper'> ");

          /* --------------------------- Timer Bug ------------------------------------------------- */
          client.print("<section class='content-header'>");
          client.print("<h1><a href='/' class='btn btn-primary btn-lg'><i class='fas fa-home'></i></a>");
          client.print("<b class='ml-3'>Diensenbeer</b> Homebrew");
          client.print("<small class='float-right'>"); 
          client.print(" <i class='fas fa-hourglass-half'></i> ");                   
          client.print(timer_bug_cont);
          client.print("% </small></h1></section>");
          /* --------------------------- FIM Timer Bug ------------------------------------------------- */ 
          
          client.print("<section class='content-header'>");
          client.print("<div class='container-fluid'>");
          client.print("<div class='row mb-2'>");
          client.print("<div class='col-sm-6'>");
          client.print("<h1> <i class='fas fa-beer'></i>  Equipamentos</h1> ");
          client.print("</div>");
          client.print("</div>");
          client.print("</div> ");
          client.print("</section>");
          
          client.print("<section class='content'> ");    
          client.print("<div class='row'>");


          /* ---------------------------------------- BRASSAGEM / PANELA ------------------------------------------------ */
          if (readString.indexOf("panela=60") > 0)
          {
            temp_config_panela=60;
          }
          else if (readString.indexOf("panela=ferver") > 0) {
            temp_config_panela=100;
          }else if (readString.indexOf("panela=off") > 0) {
            temp_config_panela=-90;
          }else if (readString.indexOf("panela=up") > 0) {
            temp_config_panela++;
          }else if (readString.indexOf("panela=down") > 0) {
            temp_config_panela--;
          }

          
          
          client.print("<div class='col-md-6'> ");
          client.print("<div class='small-box bg-gradient-warning'>");
          client.print("<div class='inner'>");
          client.print("<h3>");                                                                                                                           
          client.print(temp_sensor_panela);
          client.print("º <small mr-3>");                                                                                                                           //
          client.print(temp_config_panela);
          client.print("º</small> ");
          client.print(tempo_hora);
          client.print(":");
          client.print(tempo_minuto);
          client.print(":");
          client.print(tempo_segundo);   
                   
          client.print("</h3>");
          client.print("<p>Brassagem");          
          if(panela_ligada==true){
            client.print("<small class='text-danger float-right'><i class='fas fa-bolt'></i> Resistência ligada </small>");
          }          
          if(bomba_ligada==true){
            client.print("<small class='text-danger float-right'><i class='fas fa-shower'></i> Bomba ligada </small>");
          }
          client.print("</p>");
          client.print("</div>");
          client.print("<div class='icon'> ");
          client.print("<i class='ion ion-thermometer'></i>");
          client.print("</div>");
          client.print("<div class='card-footer col-md-12'>");
          client.print("<a href='.?panela=up' class='btn btn-default float-left'><i class='fas fa-plus-square'></i></a>");
          client.print("<a href='.?panela=down' class='btn btn-default ml-2 float-left'><i class='fas fa-minus-square'></i></a>");
          client.print("<a href='.?panela=60' class='btn btn-default float-left ml-4'>60º</a>");
          
          client.print("<a href='.?temporizador=play' class='btn btn-default float-right ml-2'><i class='far fa-play-circle text-primary'></i></a>");
          client.print("<a href='.?temporizador=pause' class='btn btn-default float-right ml-2'><i class='far fa-pause-circle text-primary'></i></a>");
          client.print("<a href='.?temporizador=restart' class='btn btn-default float-right ml-2'><i class='fas fa-redo-alt text-primary'></i></a>");
          client.print("</div>");
          client.print("<a href='?panela=off' class='small-box-footer'>Desligar Resistência <i class='fas fa-power-off'></i></a>");
          client.print("<a href='?panela=ferver' class='small-box-footer'>Ferver Panela <i class='fas fa-fire text-red'></i></a>");
          client.print("</div></div>");  

          /* ---------------------------------------- FIM BRASSAGEM / PANELA ------------------------------------------------ */


          /* -------------------------------------- Geladeira ----------------------------------------------*/

          if (readString.indexOf("geladeira=2") > 0)
          {
            temp_config_geladeira=2;
          }
          else if (readString.indexOf("geladeira=off") > 0) {
            temp_config_geladeira=57;
          }else if (readString.indexOf("geladeira=up") > 0) {
            temp_config_geladeira++;
          }else if (readString.indexOf("geladeira=down") > 0) {
            temp_config_geladeira--;
          }
          

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
          client.print("<a href='?geladeira=up' class='btn btn-primary float-left'><i class='fas fa-plus-square'></i></a>");
          client.print("<a href='?geladeira=down' class='btn btn-primary ml-2'><i class='fas fa-minus-square'></i></a>");
          client.print("<a href='.?geladeira=2' class='btn btn-default float-right'>2º</a>");
          client.print("</div>");
          client.print("<a href='?geladeira=off' class='small-box-footer'>Desligar Geladeira <i class='fas fa-power-off'></i></a>");
          client.print("<a href='?geladeira=termostato' class='small-box-footer'>Usar Termostato <i class='fas fa-temperature-low'></i></a>");
          client.print("</div></div>");

          /* ---------------------------------------- FIM GELADEIRA ------------------------------------------------ */
  
           
          client.print("</div></div></div></div>");         
  
          
          client.print("<script src='https://adminlte.io/themes/v3/plugins/jquery/jquery.min.js'></script>");
          client.print("<script src='https://adminlte.io/themes/v3/plugins/bootstrap/js/bootstrap.bundle.min.js'></script>");
          client.print("<script src='https://adminlte.io/themes/v3/dist/js/adminlte.min.js'></script>");
          client.print("<script src='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.14.0/js/all.js'></script>");
          client.print("</body>");
          client.print("</html>");
          
          
          
          readString = ""; //A VARIÁVEL É REINICIALIZADA
          client.stop(); //FINALIZA A REQUISIÇÃO HTTP E DESCONECTA O CLIENTE
        }
      }
    }
  }

  delay(10);
}
