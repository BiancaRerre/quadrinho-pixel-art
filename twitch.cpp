#include <IRCClient.h> 
#include <ESP8266HTTPClient.h>
#include "senhas.h"
#include "twitch.h"
#include "iot_settings.h"
#include "display_pixel.h"
#include <FS.h>
//The name of the channel that you want the bot to join

//tive  que colocar aqui pois não sei como está a estrutura do arquivo senhas.h
//#define TWITCH_BOT_NAME "justinfan112312312345" //usuario justinfan112312312345 é visitante, apenas leitura.
//#define TWITCH_OAUTH_TOKEN ""
bool botdebug = false;

int msparsed = 300;
int xparsed = 2;
bool setupparsed = false;
String dParsed[21];
int frameatual = 1;
int ultimoframetempo = 0;


char recebidos[21][256];

String ircChannel = "";
String mensagem = "";
WiFiClient wiFiClient;
WiFiClientSecure wifiClient;  // Use WiFiClientSecure para permitir requisições HTTPS


int controle = 0;
IRCClient client(IRC_SERVER, IRC_PORT, wiFiClient);


String parsefile(String argument) {
    // Abre o arquivo para leitura
    File file = SPIFFS.open("/payload.txt", "r");

    // Verifica se o arquivo foi aberto corretamente
    if (!file) {
        Serial.println("Falha ao abrir o arquivo.");
        return "";
    }

  




    // Tamanho máximo do conteúdo
    const size_t MAX_CONTENT_SIZE = 6000; // Ajuste conforme necessário

    // Buffer para armazenar o conteúdo
    char content[MAX_CONTENT_SIZE + 1]; // +1 para o caractere nulo de terminação da string
    
    // Lê todo o conteúdo do arquivo para o buffer
    size_t bytesRead = file.readBytes(content, MAX_CONTENT_SIZE);

    // Adiciona o caractere nulo de terminação da string
    content[bytesRead] = '\0';

    // Fecha o arquivo
    file.close();

    // Vetor de strings para armazenar partes separadas do conteúdo
    String parteSolicitada;

    // Realiza o parsing do conteúdo do arquivo
    char* part = strtok(content, "&");
    while (part != NULL) {
        String currentPart(part);
        if (currentPart.startsWith(argument + "=")) {
            parteSolicitada = currentPart.substring(currentPart.indexOf('=') + 1);
            break; // Para o loop assim que a parte solicitada for encontrada
        }
        part = strtok(NULL, "&");
    }

    if (parteSolicitada.length() > 0) {
        if (argument.indexOf('d') != -1) {
            parteSolicitada = "=" + parteSolicitada; // Adiciona o sinal de igual se for d a solicitação
        }
        
        Serial.print("Parte solicitada: ");
        Serial.println(parteSolicitada);
        // Retorna a parte correspondente como uma String
        return parteSolicitada;
    } else {
        // Se o argumento especificado não for encontrado, retorna uma string vazia
        Serial.println("Argumento não encontrado.");
        return "";
    }
}

void sendTwitchMessage(String message) {
  client.sendMessage(ircChannel, message);
}


void pixelrequest(String url){ //Função que faz o request da url encurtada pra receber a resposta
    HTTPClient http;
    wifiClient.setInsecure();
    http.begin(wifiClient, url);

    int httpCode = http.GET();
    if (httpCode > 0) {
      //Serial.printf("[HTTP] Código de resposta: %d\n", httpCode);     
      String payload = http.getString();
      //Serial.println("Resposta:");
      //Serial.println(payload);
      msg="desenho";
      desenhar(payload);//chama a função de enviar o desenho pro quadro
    if(httpCode != 200){
      controle=0; //RESETA SE O CODIGO NAO FOR 200
    }
    } else {
      Serial.printf("[HTTP] Falha na requisição. Código de erro: %d\n", httpCode);
      controle=0; //RESETA SE FALHAR

    }
   
    http.end();
}

void limpaAnimaCache(){
  for (int i = 0; i < 21; i++) {
    dParsed[i] = ""; // Obter o valor de "d[i]" usando a função parsefile    
    }
    delay(50);
}


void animarequest(String url) {
    msg = "pause";
    HTTPClient http;
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure(); // Desabilita a verificação SSL

    http.begin(wifiClient, url);

    int httpCode = http.GET();
    Serial.println("Código de resposta: "+String(httpCode));

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient *stream = http.getStreamPtr();

        // Abre o arquivo para escrita
        File file = SPIFFS.open("/payload.txt", "w");

        if (!file) {
            Serial.println("Falha ao abrir o arquivo.");
            return;
        }

        // Lê e armazena o conteúdo recebido na flash
        while (http.connected() && stream->available()) {
            char c = stream->read();
            file.write(c); // Escreve o caractere no arquivo
            Serial.write(c); // Escreve o caractere no Serial Monitor
        }

        // Fecha o arquivo
        file.close();

        Serial.println("\nPayload recebido e salvo na flash.");
    } else {
        Serial.println("⚠️Erro " +String(httpCode)+ " ao fazer a requisição da animação.⚠️");
        sendTwitchMessage("⚠️Erro " +String(httpCode)+ " ao fazer a requisição da animação!");      
        msg="reset"; //RESETA SE O CODIGO NAO FOR 200


    }
    msg="chatAnima";

    http.end();
}
 
void callback(IRCMessage ircMessage) {
  //sendTwitchMessage(ircMessage.nick);
  if (ircMessage.nick == "streamlabs") {
      if (ircMessage.text.indexOf("seja bem vindo ao Lab!") > -1){msg="follow";}
      if (ircMessage.text.indexOf("bits!") > -1 ) {msg="bits";}
      if (ircMessage.text.indexOf("mandou um Prime!") > -1){msg="prime";}
      if (ircMessage.text.indexOf("SUB") > -1){msg="sub";}
      if (ircMessage.text.indexOf("gifted") > -1){msg="presente";}
      if (ircMessage.text.indexOf("chegou") > -1){msg="raid";}
      if (ircMessage.text.indexOf("Um usuário anônimo presenteou") > -1){msg="anonimo";}
 
    //sendTwitchMessage("STREAMLABS FALOU");
  }
  if (ircMessage.text == "!mouser") {
    msg="mouser";
  }
  // if pra verificar se a url é encurtada assim https://rerre.net/PixelEditor/beta2/mini.php?url=ABOBAAAA
  
   if (ircMessage.text.indexOf("!editor") > -1){
    sendTwitchMessage("https://rerre.net/PixelEditor/");
  }
  
  if (ircMessage.text.indexOf("!bianca") > -1){
    sendTwitchMessage("https://rerre.net/contato/");
    msg="bianca";
  }

  if (ircMessage.text.indexOf("!reset") > -1){
    msg="reset";
  }

  

 
  if (ircMessage.text.startsWith("!pixel")) {
      String message = ircMessage.text;    
      char *param = strtok((char*)message.c_str(), " "); // Divide a string em tokens
      param = strtok(NULL, " "); // Avança para a próxima palavra
        if (param != NULL && strcmp(param, "config") == 0) { // verifica se a segunda palavra é "debug"
          param = strtok(NULL, " "); // Avança para a próxima palavra
          if (param != NULL && strcmp(param, "debug") == 0) { // verifica se a segunda palavra é "debug"
            botdebug = !botdebug;
            sendTwitchMessage("Debugando no chat: " + String(botdebug));
            param = strtok(NULL, " "); // Avança para a próxima palavra
          }
          if (param != NULL && strcmp(param, "brilho") == 0) { // verifica se a segunda palavra é "debug"
            param = strtok(NULL, " "); // Avança para a próxima palavra
            //Serial.println(String(param));                      
            matrix->setBrightness(String(param).toInt());
            sendTwitchMessage("Setando brilho para: "+String(param));         
          }          
        }
        if (param != NULL && strcmp(param, "parse") == 0) { // verifica se a segunda palavra é "debug"
          param = strtok(NULL, " "); // Avança para a próxima palavra
          Serial.println(String(param));
          sendTwitchMessage(parsefile(String(param)));         
        }
        if (param != NULL && strcmp(param, "show") == 0) { // verifica se a segunda palavra é "debug"
          param = strtok(NULL, " "); // Avança para a próxima palavra
          desenhar(parsefile(String(param)));
      
        }
 
  }       

  


   if (strstr(ircMessage.text.c_str(), "PixelEditor") != nullptr && //verificar se é sobre o pixeleditor
        ircMessage.text.indexOf(' ') == -1 &&//verificar se não contem espaços
        ircMessage.text.startsWith("https")//verificar se é uma url
        ){
      if(ircMessage.text.indexOf("url=") != -1) //verificar se tem o parametro url
        { 
       Serial.println("Desenho recebido!"); //informa que recebeu e detectou um link valido
       pixelrequest(ircMessage.text);  //chama a função que faz o request na url recebida
        }
      if(ircMessage.text.indexOf("anima=") != -1) //verificar se tem o anima
        { 
        limpaAnimaCache(); 
        setupparsed = false;
        Serial.println("Animação recebida!"); //informa que recebeu e detectou um link valido
        animarequest(ircMessage.text);  //chama a função que faz o request na url recebida
        }
 }
  
  
}

void setupTWconnection(){

  if (!SPIFFS.begin()) {
    Serial.println("Falha ao inicializar o sistema de arquivos SPIFFS.");
    return;
  }

  // Verificar se o arquivo payload.txt já existe
// Verificar se o arquivo payload.txt já existe
  if (!SPIFFS.exists("/payload.txt")) {
    // Se não existir, criar o arquivo payload.txt
    File file = SPIFFS.open("/payload.txt", "w");
    if (file) {
      file.close();
      Serial.println("Arquivo payload.txt criado.");
    } else {
      Serial.println("Erro ao criar o arquivo payload.txt.");
    }
  } else {
    Serial.println("FS INICIALIZADO COM SUCESSO");
  }
  
  ircChannel = "#" + twitchChannelName;
  client.setCallback(callback);

}

void loopTW(){
if (!client.connected()) {
    Serial.println("Attempting to connect to " + ircChannel );
    // Attempt to connect
    // Second param is not needed by Twtich 
//  if (client.connect(TWITCH_BOT_NAME, "", TWITCH_OAUTH_TOKEN)) {
   if (client.connect(TWITCH_BOT_NAME, "", TWITCH_OAUTH_TOKEN)) {
      client.sendRaw("JOIN " + ircChannel);
      Serial.println("connected and ready to rock");
      sendTwitchMessage(twitchInitializationMessage);
    } else {
      Serial.println("failed... try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    return;
  }
  client.loop();

}


//parte de animação ===============================




void lerArquivo(const char* filename) {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();
}


void playChat2(int tempof){
  unsigned long currentTime = millis();

    // Verifica se já passou tempo suficiente para o próximo frame
  if (currentTime - ultimoframetempo >= parsefile("ms").toInt()) {
    // Executa o próximo frame
    if (frameatual < parsefile("x").toInt()) {
      //String framename = "d"+String(frameatual);
      //senão, da flash, com limitação de velocidade
        String framename = "d"+String(frameatual);
        desenhar(parsefile(framename));
      
            
      frameatual++;
      if(frameatual == parsefile("x").toInt()){
        frameatual = 0;
      }

    } else {
      // Reinicia a animação se chegou ao último frame
      frameatual = 0;
    }
    
    // Atualiza o tempo do último frame
    ultimoframetempo = currentTime;
  }
  

}



