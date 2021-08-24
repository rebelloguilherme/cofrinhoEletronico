#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <FS.h>
#include <Servo.h>
#include <ListLib.h>
#include <Effortless_SPIFFS.h>

const int REFRESH_TIME = 300;           // time to refresh the Nextion data every 1000 ms
unsigned long refresh_timer = millis(); // timer for refreshing Nextion's page
long debouncing_time = 1500;            //Debouncing Time in Milliseconds 1000 //Software debouncing in Interrupt, by Delphiño K.M.
volatile unsigned long last_micros;
Servo servoMoeda;

//Definições dos pinos do ESP 8266
const int moeda100 = 5;
const int moeda50 = 4;
const int moeda25 = 14;
const int moeda5 = 12;
const int moeda10 = 13;

//Variaveis utilizadas no algoritmo
long t = 1000; //time between debounce interruptions
int totalPoupado = 0; //variável que representa o total poupado
int moeda = 0;
EasyNex myNex(Serial); // Create an object of EasyNex class with the name < myNex >
//some modd here
List<char> entradaSenha;
List<char> senha;
String nomeUsuario;
String obj1, obj2, obj3, obj4, obj5;
uint16_t progress1 = 0;
uint16_t progress2 = 0;
uint16_t progress3 = 0;
uint16_t progress4 = 0;
uint16_t progress5 = 0;
int saque = 0;
int valobj1Display = 0;
int valobj2Display = 0;
int valobj3Display = 0;
int valobj4Display = 0;
int valobj5Display = 0;



void Insert100()
{
  //Serial.println("Moeda de 1 real detectada");//Used if Nextion display is disabled
  moeda = 100;
  servoMoeda.write(0);
  digitalWrite(LED_BUILTIN, LOW);
}

void Insert50()
{
  //Serial.println("Moeda de 50 centavos detectada");//Used if Nextion display is disabled
  moeda = 50;
  servoMoeda.write(0);
  digitalWrite(LED_BUILTIN, LOW);
}

void Insert25()
{
  //Serial.println("Moeda de 25 centavos detectada");//Used if Nextion display is disabled
  moeda = 25;
  servoMoeda.write(0);
  digitalWrite(LED_BUILTIN, LOW);
}

void Insert10()
{
  // Serial.println("Moeda de 10 centavos detectada");//Used if Nextion display is disabled
  moeda = 10;
  servoMoeda.write(0);
  digitalWrite(LED_BUILTIN, LOW);
}

void Insert5()
{
  moeda = 5;
  servoMoeda.write(0);
  digitalWrite(LED_BUILTIN, LOW);
}


void comparaSenha()
{
  String entradaSenhaArray, senhaArray;
  entradaSenhaArray = entradaSenha.ToArray();
  senhaArray = senha.ToArray();
  if (entradaSenhaArray.equals(senhaArray))
  {
    delay(200);
    myNex.writeStr("page Unlk");
  }
  else
  {
    entradaSenha.Clear();
    entradaSenhaArray.clear();
    delay(200);
    myNex.writeStr("page Lockd");
  }
}

void IRAM_ATTR debounceInterrupt100()
{ //antes estava void ICACHE_RAM_ATTR
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert100();
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt50()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert50();
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt25()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert25();
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt10()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert10();
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt5()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert5();
    last_micros = micros();
  }
}

void setup()
{  
  myNex.begin(9600);
  pinMode(moeda100, INPUT);
  pinMode(moeda50, INPUT);
  pinMode(moeda25, INPUT);
  pinMode(moeda5, INPUT);
  pinMode(moeda10, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); // The built-in LED(GPIO 2) is initialized as an output and will be used to debug some stuff
  digitalWrite(LED_BUILTIN, HIGH);
  servoMoeda.attach(15); // attaching PIN D8(GPIO15) to servo Signal pin
  delay(200);
  servoMoeda.write(150);
  myNex.writeStr("page intro"); // For synchronizing Nextion page in case of reset to Arduino
  delay(50);
  myNex.lastCurrentPageId = 0; // At the first run of the loop, the currentPageId and the lastCurrentPageId

  attachInterrupt(digitalPinToInterrupt(moeda100), debounceInterrupt100, RISING);
  attachInterrupt(digitalPinToInterrupt(moeda50), debounceInterrupt50, RISING);
  attachInterrupt(digitalPinToInterrupt(moeda25), debounceInterrupt25, RISING);
  attachInterrupt(digitalPinToInterrupt(moeda5), debounceInterrupt10, RISING);
  attachInterrupt(digitalPinToInterrupt(moeda10), debounceInterrupt5, RISING);
}

void loop()
{
  //if ((millis() - refresh_timer) > REFRESH_TIME)
  // {
  eSPIFFS fileSystem;  
  
  //Fazer uma tela aqui que caso o sistema identifique que já tem dados na memória
  //pergunta se o usuário deseja restaurar ou quer iniciar do zero
  
  //Leitura dos valores salvos em memória
  fileSystem.openFromFile("/totalPoupado.txt", totalPoupado); //reading Data from File

  myNex.NextionListen();
  switch (myNex.currentPageId)
  {
  case 0:
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  case 1: //ChangePass
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  case 2: //userData
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      nomeUsuario = myNex.readStr("nameUser.txt");
      delay(100);
      refresh_timer = millis();
    }
    break;
  case 3: //goals
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      obj1 = myNex.readStr("goals.objetivo1.txt");
      valobj1Display = myNex.readNumber("goals.objVal1.val");
      obj2 = myNex.readStr("goals.objetivo2.txt");
      valobj2Display = myNex.readNumber("goals.objVal2.val");
      obj3 = myNex.readStr("goals.objetivo3.txt");
      valobj3Display = myNex.readNumber("goals.objVal3.val");
      refresh_timer = millis();
    }
    break;
  case 4:                                            //dashboard
    if (((millis() - refresh_timer) > REFRESH_TIME)) //condição da pagina carregada pela 1ª vez
    {
      // if (valobj1Display != 777777)
      // {                              // 777777 is the return value if the code fails to read the new value
      //   lastnumber = valobj1Display; // The chances of getting a wrong value is one in a million.
      // }
      // else if (valobj1Display == 777777)
      // {
      //   valobj1Display = lastnumber;
      // }
      // if (valobj3Display != 777777)
      // {                              // 777777 is the return value if the code fails to read the new value
      //   lastnumber2 = valobj3Display; // The chances of getting a wrong value is one in a million.
      // }
      // else if (valobj3Display == 777777)
      // {
      //   valobj3Display = lastnumber2;
      // }
      // temp = valobj1Display * 100;
      totalPoupado = totalPoupado - saque * 100;
      fileSystem.saveToFile("/totalPoupado.txt", totalPoupado); //saving data into file
      saque = 0;

      progress1 = totalPoupado / valobj1Display;
      if (progress1 >= 100)
      {
        progress1 = 100;
      }
      progress2 = totalPoupado / valobj2Display;
      if (progress2 >= 100)
      {
        progress2 = 100;
      }
      progress3 = totalPoupado / valobj3Display;
      if (progress3 >= 100)
      {
        progress3 = 100;
      }
      delay(350);
      myNex.writeNum("dashboard.totalPoupado.val", totalPoupado);
      
      myNex.writeStr("dashboard.objetivo1disp.txt", obj1);
      myNex.writeNum("dashboard.objetivo1val.val", valobj1Display * 100);
      
      myNex.writeStr("dashboard.objetivo2disp.txt", obj2);
      myNex.writeNum("dashboard.objetivo2val.val", valobj2Display * 100);
      
      myNex.writeStr("dashboard.objetivo3disp.txt", obj3);
      myNex.writeNum("dashboard.objetivo3val.val", valobj3Display * 100);
      
      myNex.writeNum("dashboard.progress1.val", progress1);
      
      myNex.writeNum("dashboard.progress2.val", progress2);
      
      myNex.writeNum("dashboard.progress3.val", progress3);

      if (moeda != 0) // atualiza a cada moeda inserida
      {               //alguma moeda foi inserida                
        totalPoupado = totalPoupado + moeda;
        fileSystem.saveToFile("/totalPoupado.txt", totalPoupado); //saving data into file

        moeda = 0;
        if (servoMoeda.read() == 0)
        {
          delay(600);
          servoMoeda.write(150);
          digitalWrite(LED_BUILTIN, HIGH);
        }
      }
      if (progress1 == 100 || progress2 == 100 || progress3 == 100)
      {
        myNex.writeStr("page congrats");
      }
      refresh_timer = millis();
    }
    break;
  case 5: //congrats
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      delay(100);
      myNex.writeStr("user.txt", nomeUsuario);
      refresh_timer = millis();
    }
    break;
  case 6: //chooseSaque
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  case 7: //chooseValor
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      myNex.writeNum("chooseValor.totalPoupado.val", totalPoupado);      
      myNex.writeNum("chooseValor.saque.val",0);
      delay(100);
      do
      {
        saque = myNex.readNumber("chooseValor.saque.val");
        delay(200);
      } while (saque <= 0);
      delay(300);
      myNex.writeStr("page goals");
      refresh_timer = millis();
    }
    break;
  case 8: //wrongPass
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  case 9: //passEnter
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  case 10: //Unlk
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  case 11: //Lockd
    if ((millis() - refresh_timer) > REFRESH_TIME)
    {
      refresh_timer = millis();
    }
    break;
  default:
    break;
  }
}

void trigger0()
{
}

void trigger1()
{ //DIGITOU 1
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('1');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger2()
{ //DIGITOU 2
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('2');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger3()
{ //DIGITOU 3
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('3');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger4()
{ //DIGITOU 4
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('4');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger5()
{ //DIGITOU 5
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('5');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger6()
{ //DIGITOU 6
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('6');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger7()
{ //DIGITOU 7
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('7');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger8()
{ //DIGITOU 8
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('8');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger9()
{ //DIGITOU 9
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('9');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger10()
{ //DIGITOU 0
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add('0');
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}

void trigger11()
{ //Evento ativado no botão Lock, page1
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  entradaSenha.Clear();
  myNex.writeStr("page intro");
}

void trigger12()
{ //Evento ativado no botão Try it, page2
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  entradaSenha.Clear();
  myNex.writeStr("page passEnter");
}

void trigger13() //23 02 54 0D ação do botão sacar total na pagina chooseSaque
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  totalPoupado = 0;
  fileSystem.saveToFile("/totalPoupado.txt", totalPoupado); //saving data into file
  valobj1Display = 0;
  obj1 = "";
  obj2 = "";
  obj3 = "";
  valobj1Display = 0;
  valobj2Display = 0;
  valobj3Display = 0;
  // myNex.writeStr("goals.objetivo1.txt",obj1);
  // myNex.writeNum("goals.objVal1.val",mostraValObj1);
  myNex.writeStr("page goals");
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger14() // 23 02 54 0E ação do botão sacar valor na pagina chooseSaque
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  myNex.writeStr("page chooseValor");
  digitalWrite(LED_BUILTIN, HIGH);
}