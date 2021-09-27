#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <FS.h>
#include <Servo.h>
#include <ListLib.h>
#include <Effortless_SPIFFS.h>

#define DATA_REFRESH_RATE 1000             // The time between each Data refresh of the page, defaut is 1000
unsigned long pageRefreshTimer = millis(); // Timer for DATA_REFRESH_RATE
bool newPageLoaded = false;                // true when the page is first loaded ( lastCurrentPageId != currentPageId )

Servo servoMoeda;

//Definições dos pinos do ESP 8266
const int MOEDA100 = 5;
const int MOEDA50 = 4;
const int MOEDA25 = 14;
const int MOEDA5 = 12;
const int MOEDA10 = 13;

//Variaveis utilizadas no algoritmo
long debouncing_time = 1500; //Debouncing Time in Milliseconds 1000 //Software debouncing in Interrupt, by Delphiño K.M.//1500 era o ultimo valor
volatile unsigned long last_micros;
long t = 1000; //time between debounce interruptions
long debouncing_Servo = 500;
long tyneT = 500;

int totalPoupado = 0; //variável que representa o total poupado
int lastTotalPoupado = 0;

int Moeda = 0;
bool moedaInserida = false;
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

void refreshPage5() //congrats
{
  //mostrar o nome do usuário..
  myNex.writeStr("congrats.user.txt", nomeUsuario);
}
void refreshPage7() //chooseSaque
{
  //mostrar total poupado
  myNex.writeNum("chooseValor.totalPoupado.val", totalPoupado);
  //implementar o quanto a pessoa quer sacar...
}
void refreshPage8() //passEnter
{
}

void atualizaDashboard()
{
  myNex.writeNum("dashboard.totalPoupado.val", totalPoupado);

  myNex.writeStr("dashboard.objetivo1disp.txt", obj1);
  myNex.writeNum("dashboard.objetivo1val.val", valobj1Display * 100);

  myNex.writeStr("dashboard.objetivo2disp.txt", obj2);
  myNex.writeNum("dashboard.objetivo2val.val", valobj2Display * 100);

  myNex.writeStr("dashboard.objetivo3disp.txt", obj3);
  myNex.writeNum("dashboard.objetivo3val.val", valobj3Display * 100);

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
  myNex.writeNum("dashboard.progress1.val", progress1);
  myNex.writeNum("dashboard.progress2.val", progress2);
  myNex.writeNum("dashboard.progress3.val", progress3);
}

void Insert(int moeda) //atualiza tudo que decorre da inserção da moeda...
{
  Moeda = moeda;
  servoMoeda.write(0); //Libera a moeda
  digitalWrite(LED_BUILTIN, LOW);
  totalPoupado = totalPoupado + Moeda;
  //fileSystem.saveToFile("/totalPoupado.txt", totalPoupado); //saving data into file
  Moeda = 0;
  digitalWrite(LED_BUILTIN, HIGH);
  

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
  if (progress1 == 100 && progress2 == 100 && progress3 == 100)
  {
    myNex.writeStr("page congrats");
  }
  atualizaDashboard();
  moedaInserida = true;
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
  servoMoeda.write(140); //Rampa de moedas travada, valor original 150
  delay(250);
  myNex.writeStr("page 0"); // For synchronizing Nextion page in case of reset to Arduino
  delay(50);
  myNex.lastCurrentPageId = 1; // At the first run of the loop, the currentPageId and the lastCurrentPageId
                               // must have different values, due to run the function firstRefresh()

  attachInterrupt(digitalPinToInterrupt(MOEDA100), debounceInterrupt100, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA50), debounceInterrupt50, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA25), debounceInterrupt25, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA5), debounceInterrupt10, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA10), debounceInterrupt5, RISING);
}

void loop()
{
  myNex.NextionListen(); //Tentar deixar somente esta função no void loop
  if (moedaInserida)
  {
    if ((long)(micros() - last_micros) >= debouncing_Servo * tyneT)
    {
      servoMoeda.write(140);
      last_micros = micros();
      moedaInserida = false;
    }
  }
}

void trigger0() //introConfig configButton
{
  myNex.writeStr("page userData");
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
  //atualizar aqui as barras de progresso...
  progress1 = 0;
  progress2 = 0;
  progress3 = 0;

  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page goals");
}

void trigger14() //chooseSaque sacarValor(button)
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page chooseValor");
  myNex.writeNum("chooseValor.totalPoupado.val", totalPoupado);
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
  nomeUsuario = myNex.readStr("userData.nameUser.txt");
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page goals");
}

void trigger18() //goals okButton
{
  obj1 = myNex.readStr("goals.objetivo1.txt");
  valobj1Display = myNex.readNumber("goals.objVal1.val");
  fileSystem.saveToFile("/valobj1Display.txt", valobj1Display);
  obj2 = myNex.readStr("goals.objetivo2.txt");
  valobj2Display = myNex.readNumber("goals.objVal2.val");
  fileSystem.saveToFile("/valobj2Display.txt", valobj2Display);
  obj3 = myNex.readStr("goals.objetivo3.txt");
  valobj3Display = myNex.readNumber("goals.objVal3.val");
  fileSystem.saveToFile("/valobj3Display.txt", valobj3Display);

  delay(50);
  fileSystem.saveToFile("/obj1.txt", obj1); //saving data into file
  delay(50);
  fileSystem.saveToFile("/obj2.txt", obj2); //saving data into file
  delay(50);
  fileSystem.saveToFile("/obj3.txt", obj3); //saving data into file
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  atualizaDashboard();
  myNex.writeStr("page dashboard");
  //chama função que atualiza dashboard
}

void trigger19() //goals cancelButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page animationTest"); //only for test purpouses
}

void trigger20() //Dashboard backButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page goals");
}

void trigger21() //Dashboard unlockButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page passEnter");
}

void trigger22() //congrats openButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page chooseSaque");
}

void trigger23() //congrats waitButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
}

void trigger24() //chooseValor cancelButton
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page chooseSaque");
}

void trigger25() //chooseValor okButton
{
  
  saque = myNex.readNumber("chooseValor.saque.val");
  
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  totalPoupado = totalPoupado - (saque*100);
  myNex.writeStr("page dashboard");
  atualizaDashboard();
}