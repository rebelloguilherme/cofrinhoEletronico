#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <FS.h>
#include <Servo.h>
#include <ListLib.h>
#include <Effortless_SPIFFS.h>

#define DATA_REFRESH_RATE 1000             // The time between each Data refresh of the page
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
long debouncing_time = 1500; //Debouncing Time in Milliseconds 1000 //Software debouncing in Interrupt, by Delphiño K.M.
volatile unsigned long last_micros;
long t = 1000; //time between debounce interruptions

int totalPoupado = 0; //variável que representa o total poupado
int lastTotalPoupado = 0;

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

  listeningMoeda();

  refreshCurrentPage();

  firstRefresh();
}

void listeningMoeda()
{
  if (Moeda != 0)
  {
    //alguma moeda foi inserida
    totalPoupado = totalPoupado + Moeda;
    fileSystem.saveToFile("/totalPoupado.txt", totalPoupado); //saving data into file
    Moeda = 0;

    if (servoMoeda.read() == 0)
    {
      delay(600);
      servoMoeda.write(150);
      digitalWrite(LED_BUILTIN, HIGH);
    }

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
  }
}
void firstRefresh()
{ // This function's purpose is to update the values of a new page when is first loaded,
  // and refreshing all the components with the current values as Nextion shows the Attribute val.

  if (myNex.currentPageId != myNex.lastCurrentPageId)
  { // If the two variables are different, means a new page is loaded.

    newPageLoaded = true; // A new page is loaded
                          // This variable is used as an argument at the if() statement on the refreshPageXX() voids,
                          // in order when is true to update all the values on the page with their current values
                          // with out run a comparison with the last value.

    switch (myNex.currentPageId)
    {
    case 5:
      refreshPage5();
      break;

    case 6:
      refreshPage6();
      break;

    case 8:
      refreshPage8();
      break;

    case 9:
      refreshPage9();
      break;
    }

    newPageLoaded = false; // After we have updated the new page for the first time, we update the variable to false.
                           // Now the values updated ONLY if the new value is different from the last Sent value.
                           // See void refreshPage0()

    myNex.lastCurrentPageId = myNex.currentPageId; // After the refresh of the new page We make them equal,
                                                   // in order to identify the next page change.
  }
}

void refreshCurrentPage()
{
  // In this function we refresh the page currently loaded every DATA_REFRESH_RATE
  if ((millis() - pageRefreshTimer) > DATA_REFRESH_RATE)
  {
    switch (myNex.currentPageId)
    {
    case 5:
      refreshPage5();
      break;

    case 6:
      refreshPage6();
      break;

    case 8:
      refreshPage8();
      break;

    case 9:
      refreshPage9();
      break;
    }
    pageRefreshTimer = millis();
  }
}

// void refreshPage0()
// {
//   // Use lastSentTemperature, in order to update the components ONLY when their value has changed
//   // and avoid sending unnecessary data over Serial.
//   // Also with the newPageLoaded boolean variable, we bypass the if() argument of temperature != lastSentTemperature (last value comparison)
//   // so as to update all the values on Nextion when a new page is loaded, independant of if the values have changed

//   if (temperature != lastSentTemperature || newPageLoaded == true)
//   {

//     String tempString = String(temperature, 1); // Convert the float value to String, in order to send it to t0 textbox on Nextion
//     myNex.writeStr("t0.txt", tempString);       //Write the String value to t0 Textbox component

//     int tempInt = temperature * 10; // We convert the float to int, in order to send it to x0 Xfloat component on Nextion
//                                     // We multiply it x10, because Xfloat will put a comma automatically after the last digit
//                                     // if vvs1 is set to 1
//     myNex.writeNum("x0.val", tempInt);

//     lastSentTemperature = temperature; // We store the last value that we have sent on Nextion, we wait for the next comparison
//                                        // and send data only when the value of temperature changes
//   }
// }

void refreshPage5()
{
  // Use lastSentTemperature, in order to update the components ONLY when their value has changed
  // and avoid sending unnecessary data over Serial.
  // Also with the newPageLoaded boolean variable, we bypass the if() argument of temperature != lastSentTemperature (last value comparison)
  // so as to update all the values on Nextion when a new page is loaded, independant of if the values have changed

  if (totalPoupado != lastTotalPoupado || newPageLoaded == true)
  {
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

    lastTotalPoupado = totalPoupado;
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