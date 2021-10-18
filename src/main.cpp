#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <FS.h>
#include <Servo.h>
#include <Effortless_SPIFFS.h>
//Definições dos pinos do ESP 8266
const int MOEDA100 = 5;
const int MOEDA50 = 4;
const int MOEDA25 = 14;
const int MOEDA5 = 12;
const int MOEDA10 = 13;

eSPIFFS fileSystem; //criando instancia da Classe eSPIFFS
Servo servoMoeda;
Servo servoPorta;
long debouncing_time = 1500; //Debouncing Time in Milliseconds 1000 //Software debouncing in Interrupt, by Delphiño K.M.//1500 era o ultimo valor
volatile unsigned long last_micros;
long t = 1000; //time between debounce interruptions
long debouncing_Servo = 500;
long tyneT = 500;

int totalPoupado = 0; //variável que representa o total poupado
int Moeda = 0;
bool moedaInserida = false;
EasyNex myNex(Serial); // Create an object of EasyNex class with the name < myNex >
//some modd here
String entradaSenha;
String senha = "0000";
String nomeUsuario = "";
String obj1;
String obj2;
String obj3;
int valobj1Display = 0;
int valobj2Display = 0;
int valobj3Display = 0;
int valTotalDisplay = 0;
uint16_t progress1 = 0;
uint16_t progress2 = 0;
uint16_t progress3 = 0;
uint16_t progressTotal = 0;
int saque = 0;

void abrirPorta()
{
  servoPorta.write(0);
}

void fecharPorta()
{
  servoPorta.write(180);
}

void ZerarValores()
{
  totalPoupado = 0;
  obj1 = "";
  obj2 = "";
  obj3 = "";
  valobj1Display = 0;
  valobj2Display = 0;
  valobj3Display = 0;
  valTotalDisplay = 0;
  progress1 = 0;
  progress2 = 0;
  progress3 = 0;
  progressTotal =0;
}

void SalvaValores()
{
  fileSystem.saveToFile("/user.txt", nomeUsuario);
  fileSystem.saveToFile("/obj1.txt", obj1);
  fileSystem.saveToFile("/obj2.txt", obj2);
  fileSystem.saveToFile("/obj3.txt", obj3);
  fileSystem.saveToFile("/valobj1Display.txt", valobj1Display);
  fileSystem.saveToFile("/valobj2Display.txt", valobj2Display);
  fileSystem.saveToFile("/valobj3Display.txt", valobj3Display);
  fileSystem.saveToFile("/valTotalDisplay.txt", valTotalDisplay);
  
}
void CarregaValores()
{
  fileSystem.openFromFile("/user.txt", nomeUsuario);
  fileSystem.openFromFile("/obj1.txt", obj1);
  fileSystem.openFromFile("/obj2.txt", obj2);
  fileSystem.openFromFile("/obj3.txt", obj3);
  fileSystem.openFromFile("/valobj1Display.txt", valobj1Display);
  fileSystem.openFromFile("/valobj2Display.txt", valobj2Display);
  fileSystem.openFromFile("/valobj3Display.txt", valobj3Display);  
  fileSystem.openFromFile("/valTotalDisplay.txt", valTotalDisplay);


}
void SalvaSaldo()
{
  fileSystem.saveToFile("/totalPoupado.txt", totalPoupado);
}
void CarregaSaldo()
{
  fileSystem.openFromFile("/totalPoupado.txt", totalPoupado);
}

void PiscaStatus()
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
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

  valTotalDisplay = valobj1Display + valobj2Display + valobj3Display;
  myNex.writeNum("dashboard.objetivoTotal.val", valTotalDisplay * 100);

  progress1 = totalPoupado / valobj1Display;
  if(progress1 < 100)
  {
    myNex.writeNum("dashboard.resgatar1.pic", 53);
    myNex.writeNum("dashboard.resgatar1.pic2", 53);
  }
  if (progress1 >= 100)
  {
    progress1 = 100;
    myNex.writeNum("dashboard.resgatar1.pic", 15);
    myNex.writeNum("dashboard.resgatar1.pic2", 16);
  }
  progress2 = totalPoupado / valobj2Display;
  if(progress2 < 100)
  {
    myNex.writeNum("dashboard.resgatar2.pic", 53);
    myNex.writeNum("dashboard.resgatar2.pic2", 53);
  }
  if (progress2 >= 100)
  {
    progress2 = 100;
    myNex.writeNum("dashboard.resgatar2.pic", 15);
    myNex.writeNum("dashboard.resgatar2.pic2", 16);
  }
  progress3 = totalPoupado / valobj3Display;
  if(progress3 < 100)
  {
    myNex.writeNum("dashboard.resgatar3.pic", 53);
    myNex.writeNum("dashboard.resgatar3.pic2", 53);
  }
  if (progress3 >= 100)
  {
    progress3 = 100;
    myNex.writeNum("dashboard.resgatar3.pic", 15);
    myNex.writeNum("dashboard.resgatar3.pic2", 16);
  }
  progressTotal = totalPoupado / valTotalDisplay;
  if(progressTotal < 100)
  {
    myNex.writeNum("dashboard.resgatarTotal.pic", 53);
    myNex.writeNum("dashboard.resgatarTotal.pic2", 53);
  }
  if (progressTotal >= 100)
  {
    progressTotal = 100;
    myNex.writeNum("dashboard.resgatarTotal.pic", 15);
    myNex.writeNum("dashboard.resgatarTotal.pic2", 16);
  }
  //escrever na tela o valor total
  myNex.writeNum("dashboard.progress1.val", progress1);
  myNex.writeNum("dashboard.progress2.val", progress2);
  myNex.writeNum("dashboard.progress3.val", progress3);
  myNex.writeNum("dashboard.progressTotal.val", progressTotal);
}

void Insert(int moeda) //atualiza tudo que decorre da inserção da moeda...
{
  Moeda = moeda;
  servoMoeda.write(0); //Libera a moeda
  totalPoupado = totalPoupado + Moeda;
  SalvaSaldo();
  Moeda = 0;
  PiscaStatus();
  atualizaDashboard();
  moedaInserida = true;
}

void comparaSenha()
{
  myNex.writeNum("passEnter.q1.picc", 4); //Apaga o 1º led virtual
  myNex.writeNum("passEnter.q2.picc", 4); //Apaga o 2º led virtual
  myNex.writeNum("passEnter.q3.picc", 4); //Apaga o 3º led virtual
  myNex.writeNum("passEnter.q0.picc", 4); //Apaga o 4º led virtual
  if (entradaSenha.equals(senha))
  {
    delay(200);
    myNex.writeStr("page unlocked");
  }
  else
  {
    entradaSenha.remove(0, 4);
    delay(200);
    myNex.writeStr("page locked");
  }
}

void Digitou(String tecla)
{ //DIGITOU tecla
  if (entradaSenha.length() < senha.length())
  {
    entradaSenha += tecla;
    if (entradaSenha.length() == 1)
    {
      myNex.writeNum("passEnter.q0.picc", 4); //ascende o 1º led virtual
      PiscaStatus();
    }
    if (entradaSenha.length() == 2)
    {
      myNex.writeNum("passEnter.q1.picc", 4); //ascende o 2º led virtual
      PiscaStatus();
    }
    if (entradaSenha.length() == 3)
    {
      myNex.writeNum("passEnter.q2.picc", 4); //ascende o 3º led virtual
      PiscaStatus();
    }
    if (entradaSenha.length() == 4)
    {
      myNex.writeNum("passEnter.q3.picc", 4); //ascende o 4º led virtual
      PiscaStatus();
    }
  }
  else if (entradaSenha.length() == senha.length())
  {
    comparaSenha();
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
  servoPorta.attach(0); //attaching PIN D3(GPIO0) to servoPorta signal Pin
  delay(200);
  
  delay(5000);
  


  servoMoeda.attach(15); // attaching PIN D8(GPIO15) to servoMoeda Signal pin
  delay(200);
  servoMoeda.write(140); //Rampa de moedas travada, valor original 150
  delay(250);
  myNex.writeStr("page 0"); // For synchronizing Nextion page in case of reset to Arduino
  delay(50);  
  attachInterrupt(digitalPinToInterrupt(MOEDA100), debounceInterrupt100, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA50), debounceInterrupt50, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA25), debounceInterrupt25, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA5), debounceInterrupt10, RISING);
  attachInterrupt(digitalPinToInterrupt(MOEDA10), debounceInterrupt5, RISING);
}

void loop()
{
  myNex.NextionListen(); //Tentar deixar somente esta função no void loop
  //criar condição de restaurar caso exista dados salvos em memória..
  //pode ser mostrado para o usuário uma tela de "Existem dados salvos em memória, deseja restaurar?"
  //chamar funcção de verificar o tamanho da memória do esp 
  //usando a lib EFFortless_SPIFFS
  //https://github.com/thebigpotatoe/Effortless-SPIFFS

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
  CarregaSaldo();
  CarregaValores();
  if (valTotalDisplay > 0 || totalPoupado > 0)
  {
    //vai pra pagina de decisão, se quer carregar os dados da memória ou não..
    myNex.writeStr("page loadMemory");
  }
  else
  {
    myNex.writeStr("page userData");
  }  
}
void trigger1()
{ //DIGITOU 1
  Digitou("1");
}
void trigger2()
{ //DIGITOU 2
  Digitou("2");
}
void trigger3()
{ //DIGITOU 3
  Digitou("3");
}
void trigger4()
{ //DIGITOU 4
  Digitou("4");
}
void trigger5()
{ //DIGITOU 5
  Digitou("5");
}
void trigger6()
{ //DIGITOU 6
  Digitou("6");
}
void trigger7()
{ //DIGITOU 7
  Digitou("7");
}
void trigger8()
{ //DIGITOU 8
  Digitou("8");
}
void trigger9()
{ //DIGITOU 9
  Digitou("9");
}
void trigger10()
{ //DIGITOU 0
  Digitou("0");
}

void trigger11() //unlocked lockButton
{                //Evento ativado no botão Lock, page1
  PiscaStatus();
  entradaSenha.remove(0, 4);
  myNex.writeStr("page intro");
}

void trigger12() //locked tryAgainButton
{
  PiscaStatus();
  entradaSenha.remove(0, 4);
  myNex.writeStr("page passEnter");
}

void trigger13() //chooseSaque sacarTotal(button)
{
}

void trigger14() //chooseSaque sacarValor(button)
{
}

void trigger15() //changePass okButton
{
  PiscaStatus();
}

void trigger16() //changePass cancelButton
{
  PiscaStatus();
}

void trigger17() //userData nextButton
{
  nomeUsuario = myNex.readStr("userData.nameUser.txt");
  PiscaStatus();
  myNex.writeStr("page goals");
}

void trigger18() //goals okButton
{
  obj1 = myNex.readStr("goals.objetivo1.txt");
  valobj1Display = myNex.readNumber("goals.objVal1.val");
  obj2 = myNex.readStr("goals.objetivo2.txt");
  valobj2Display = myNex.readNumber("goals.objVal2.val");
  obj3 = myNex.readStr("goals.objetivo3.txt");
  valobj3Display = myNex.readNumber("goals.objVal3.val");
  valTotalDisplay = valobj1Display + valobj2Display + valobj3Display;
  SalvaValores();
  PiscaStatus();  
  myNex.writeStr("page dashboard");
  atualizaDashboard();
}

void trigger19() //goals cancelButton
{
  PiscaStatus();
}

void trigger20() //Dashboard backButton
{
  PiscaStatus();
  myNex.writeStr("page goals");
}

void trigger21() //Dashboard unlockButton
{
  PiscaStatus();
  myNex.writeStr("page passEnter");
}

void trigger22() //loadMemory okButton
{
  CarregaValores();
  CarregaSaldo();
  atualizaDashboard();
  myNex.writeStr("page dashboard");
}

void trigger23() //loadMemory noButton
{
  ZerarValores();
  myNex.writeStr("page userData");
}

void trigger24() //chooseValor cancelButton
{
}

void trigger25() //chooseValor okButton
{
}

void trigger26() //dashboard resgatar1 Button
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  totalPoupado = totalPoupado - (valobj1Display * 100);
  SalvaSaldo();
  digitalWrite(LED_BUILTIN, HIGH);  
  myNex.writeStr("page congrats");
  myNex.writeStr("congrats.user.txt", nomeUsuario);
  delay(5000);  
  myNex.writeStr("page dashboard");
  atualizaDashboard();
}
void trigger27() //dashboard resgatar2 Button
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  totalPoupado = totalPoupado - (valobj2Display * 100);
  SalvaSaldo();
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page congrats");  
  myNex.writeStr("congrats.user.txt", nomeUsuario);  
  delay(5000);
  myNex.writeStr("page dashboard");
  atualizaDashboard();  
}
void trigger28() //dashboard resgatar3 Button
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  totalPoupado = totalPoupado - (valobj3Display * 100);
  SalvaSaldo();
  digitalWrite(LED_BUILTIN, HIGH);
  myNex.writeStr("page congrats");  
  myNex.writeStr("congrats.user.txt", nomeUsuario);  
  delay(5000);
  myNex.writeStr("page dashboard");
  atualizaDashboard();  
}
void trigger29() //dashboard resgatarTotal Button
{  
  ZerarValores();
  SalvaSaldo();  
  SalvaValores();
  atualizaDashboard();
  PiscaStatus();  
  myNex.writeStr("congrats.user.txt", nomeUsuario);  
  myNex.writeStr("page congrats");   
  delay(5000);
  myNex.writeStr("page goals");  
}