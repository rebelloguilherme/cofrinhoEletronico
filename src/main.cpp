#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <FS.h>
#include <Servo.h>
#include <ListLib.h>
#include <Effortless_SPIFFS.h>


const int REFRESH_TIME = 300;           // time to refresh the Nextion data every x ms
unsigned long refresh_timer = millis(); // timer for refreshing Nextion's page
long debouncing_time = 1500;            //Debouncing Time in Milliseconds 1000 //Software debouncing in Interrupt, by Delphiño K.M.
volatile unsigned long last_micros;
Servo servoMoeda;

//Definições dos pinos do ESP 8266
const int MOEDA100 = 5;
const int MOEDA50 = 4;
const int MOEDA25 = 14;
const int MOEDA5 = 12;
const int MOEDA10 = 13;

//Variaveis utilizadas no algoritmo
long t = 1000;        //time between debounce interruptions
int totalPoupado = 0; //variável que representa o total poupado
int Moeda = 0;
EasyNex myNex(Serial); // Create an object of EasyNex class with the name < myNex >
//some modd here
List<char> entradaSenha;
List<char> senha;
String nomeUsuario = "";
String obj1;
String obj2;
String obj3;
int valobj1Display = 0;
int valobj2Display = 0;
int valobj3Display = 0;
uint16_t progress1 = 0;
uint16_t progress2 = 0;
uint16_t progress3 = 0;
int saque = 0;
eSPIFFS fileSystem; //criando instancia da Classe eSPIFFS
bool carregarDados = true;

void Insert(int moeda)
{
  //Serial.println("Moeda de 1 real detectada");//Used if Nextion display is disabled
  Moeda = moeda;
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
    Insert(100);
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt50()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert(50);
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt25()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert(25);
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt10()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert(10);
    last_micros = micros();
  }
}

void IRAM_ATTR debounceInterrupt5()
{
  if ((long)(micros() - last_micros) >= debouncing_time * t)
  {
    Insert(5);
    last_micros = micros();
  }
}

void setup()
{
  myNex.begin(9600);
  pinMode(MOEDA100, INPUT);
  pinMode(MOEDA50, INPUT);
  pinMode(MOEDA25, INPUT);
  pinMode(MOEDA5, INPUT);
  pinMode(MOEDA10, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); // The built-in LED(GPIO 2) is initialized as an output and will be used to debug some stuff
  digitalWrite(LED_BUILTIN, HIGH);
  servoMoeda.attach(15); // attaching PIN D8(GPIO15) to servo Signal pin
  delay(200);
  servoMoeda.write(150);
  myNex.writeStr("page intro"); // For synchronizing Nextion page in case of reset to Arduino
  delay(50);
  myNex.lastCurrentPageId = 0; // At the first run of the loop, the currentPageId and the lastCurrentPageId

  attachInterrupt(digitalPinToInterrupt(MOEDA100), debounceInterrupt100, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA50), debounceInterrupt50, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA25), debounceInterrupt25, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA5), debounceInterrupt10, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA10), debounceInterrupt5, RISING);
}

void loop()
{
  myNex.NextionListen(); //Tentar deixar somente esta função no void loop
}

void trigger0() //introConfig configButton
{
  myNex.writeStr("page userData");
}

void adicionaSenha(char c)
{
  if (entradaSenha.Count() < senha.Count())
  {
    entradaSenha.Add(c);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (entradaSenha.Count() == senha.Count())
  {
    comparaSenha();
  }
}
void trigger1()
{ //DIGITOU 1
  adicionaSenha('1');
}

void trigger2()
{ //DIGITOU 2
  adicionaSenha('2');
}

void trigger3()
{ //DIGITOU 3
  adicionaSenha('3');
}

void trigger4()
{ //DIGITOU 4
  adicionaSenha('4');
}

void trigger5()
{ //DIGITOU 5
  adicionaSenha('5');
}

void trigger6()
{ //DIGITOU 6
  adicionaSenha('6');
}

void trigger7()
{ //DIGITOU 7
  adicionaSenha('7');
}

void trigger8()
{ //DIGITOU 8
  adicionaSenha('8');
}

void trigger9()
{ //DIGITOU 9
  adicionaSenha('9');
}

void trigger10()
{ //DIGITOU 0
  adicionaSenha('0');
}

void trigger11() //unlocked lockButton
{                //Evento ativado no botão Lock, page1
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  entradaSenha.Clear();
  myNex.writeStr("page intro");
}

void trigger12() //locked tryAgainButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  entradaSenha.Clear();
  myNex.writeStr("page passEnter");
}

void trigger13() //chooseSaque sacarTotal(button)
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

void trigger14() //chooseSaque sacarValor(button)
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  myNex.writeStr("page chooseValor");
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger15() //changePass okButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger16() //changePass cancelButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger17() //userData nextButton
{
  myNex.writeStr("page goals");
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger18() //goals okButton
{
  myNex.writeStr("page dashboard");
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger19() //goals cancelButton
{
  myNex.writeStr("page animationTest"); //only for test purpouses
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger20() //Dashboard backButton
{
  digitalWrite(LED_BUILTIN, LOW);
  myNex.writeStr("page goals");
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger21() //Dashboard unlockButton
{
  digitalWrite(LED_BUILTIN, LOW);
  myNex.writeStr("page passEnter");
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger22() //congrats openButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger23() //congrats waitButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger24() //chooseValor cancelButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}