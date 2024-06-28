#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Stepper.h>
#include <Servo.h>

#define SERVO2 11   //pino D1 da shield (PWM)
#define STEP 12     //pino D2 da shield
#define DIR 13      //pino D3 da shield
#define ENABLE 1    //pino D5 da shield
#define SENSOR 2    //pino D6 da shield
#define SERVO1 3    //pino D7 da shield (PWM)
#define PASSOS 200  //passos do motor

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Servo S1;
Servo S2;
Stepper motor(PASSOS, STEP, DIR, ENABLE, 0);

int keypad_pin = A0;
int keypad_value = 0;
int keypad_value_old = 0;
int count = 1;
int maxCortes = 300;
int valorDesejado = 0;
 
unsigned long previousMillis = 0;
const unsigned long tempoPressionado = 2000;
const long interval = 500;

bool displayCount = true;
bool incrementando = false;
bool tarefaIniciada = false;

char btn_push;

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 3;

void MainMenuDisplay(){
    lcd.clear();
    lcd.setCursor(0,0);
    switch (mainMenuPage)
    {
        case 1:
          lcd.print("1. Resistor 330R");
          break;
        case 2:
          lcd.print("2. Res. 1K/10K");
          break;
        case 3:
          lcd.print("3. Diodo 1N4007");
    }
}

char ReadKeypad(){
  /* Keypad button analog Value
  no button pressed 1023
  select  741
  left    503
  down    326
  up      142
  right   0 
  */
  keypad_value = analogRead(keypad_pin);
  
  if(keypad_value < 100)
    return 'R';
  else if(keypad_value < 200)
    return 'U';
  else if(keypad_value < 400)
    return 'D';
  else if(keypad_value < 600)
    return 'L';
  else if(keypad_value < 800)
    return 'S';
  else 
    return 'N';
 
}

void iniciarTarefa(int menu, int qtde) {
  int qtdeSensor = 0;
  int debouncing = 100;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando tarefa...");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Tarefa iniciada!");

  while (qtdeSensor < qtde+1) {
    motor.step(20);
    delay(5);
    if (digitalRead(2) == HIGH) {
      qtdeSensor++;
      lcd.setCursor(0, 1);
      lcd.print("Resistor Detectado!");
      while (digitalRead(2) == HIGH) {
        delay(debouncing);
      }
      for (int i = maxCortes; i >= 0; i--) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Restantes: ");
      lcd.print(i);
      }
      if (qtdeSensor == qtde) {
        motor.step(0);
        qtdeSensor = 0;
      }
      delay(1000); // Delay para visualização do texto
      lcd.clear(); // Limpa o display após detecção
      }
    }
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Tarefa concluida!");
}

bool tarefaCompletada() {
    // Implemente a lógica para verificar se a tarefa foi completada
    // Exemplo simples: aguarda um tempo fixo para simular o término da tarefa
    delay(2000); // Exemplo: aguarda 1 segundo para simular término da tarefa
    lcd.clear();
    return true; // Retorna verdadeiro para indicar que a tarefa foi completada
}

void Menu1() {
  int menuID = 1;  
    while (true) {
        lcd.setCursor(0, 1);
        lcd.print("Qtde Cortes:");
        lcd.setCursor(13, 1);
        lcd.print(count);

        bool botaoConfirmarPressionado = false; // Flag para verificar se o botão de confirmar foi pressionado

        while (ReadKeypad() != 'L') {
            unsigned long currentMillis = millis();
            
            // Verifica se alcançou o valor desejado e para de piscar
            if (count == valorDesejado && !tarefaIniciada && incrementando) {
                displayCount = true; // Garante que o count seja exibido
                previousMillis = currentMillis; // Reinicia o timer de piscar
            }
            
            // Piscar somente se não estiver incrementando, e se não tiver alcançado o valor desejado
            if (!incrementando && !tarefaIniciada && currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                displayCount = !displayCount;
                lcd.setCursor(13, 1);
                if (displayCount) {
                    lcd.print(count);
                } else {
                    lcd.print("    "); // Apaga o valor
                }
            }

            char key = ReadKeypad();
            if (key == 'U' && !tarefaIniciada) {
                incrementando = true;
                count++;
                if (count > maxCortes){
                    count = 1; // Retorna a 1 se ultrapassar o máximo
                }
                lcd.setCursor(13, 1);
                lcd.print(count);
                delay(150); // Debounce
                displayCount = true; // Garante que o count seja exibido imediatamente após incremento
            } else if (key == 'D' && !tarefaIniciada) {
                incrementando = true;
                count--;
                if (count < 1){
                    count = maxCortes; // Retorna a maxCortes se for menor que 1
                }
                lcd.setCursor(13, 1);
                lcd.print(count);
                delay(150); // Debounce
                displayCount = true; // Garante que o count seja exibido imediatamente após decremento
            } else {
                incrementando = false;
            }
            
            // Verifica se o botão 'S' foi pressionado por tempoPressionado para iniciar a tarefa
            if (key == 'S' && !tarefaIniciada) {
                unsigned long pressStartMillis = millis();
                while (ReadKeypad() == 'S') {
                    if (millis() - pressStartMillis >= tempoPressionado) {
                        valorDesejado = count; // Define o valor desejado como o valor atual de count
                        iniciarTarefa(menuID, 15); // Passa o menuID para iniciarTarefa
                        tarefaIniciada = true;
                        botaoConfirmarPressionado = true; // Marca o botão de confirmar como pressionado
                        break;
                    }
                }
            }
            
            // Se o botão de confirmar foi pressionado e a tarefa foi completada, reinicia o menu
            if (botaoConfirmarPressionado && tarefaCompletada()) {
                tarefaIniciada = false; // Reinicia o loop do Menu1
                count = 1; // Reseta count para começar de novo
                valorDesejado = 0; // Reseta o valor desejado
                MainMenuDisplay(); // Retorna ao menu principal
                return; // Sai do loop interno para retornar ao MainMenuDisplay()
            }
        }
    }
}

void Menu2() {
  int menuID = 2;  
    while (true) {
        lcd.setCursor(0, 1);
        lcd.print("Qtde Cortes:");
        lcd.setCursor(13, 1);
        lcd.print(count);

        bool botaoConfirmarPressionado = false; // Flag para verificar se o botão de confirmar foi pressionado

        while (ReadKeypad() != 'L') {
            unsigned long currentMillis = millis();
            
            // Verifica se alcançou o valor desejado e para de piscar
            if (count == valorDesejado && !tarefaIniciada && incrementando) {
                displayCount = true; // Garante que o count seja exibido
                previousMillis = currentMillis; // Reinicia o timer de piscar
            }
            
            // Piscar somente se não estiver incrementando, e se não tiver alcançado o valor desejado
            if (!incrementando && !tarefaIniciada && currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                displayCount = !displayCount;
                lcd.setCursor(13, 1);
                if (displayCount) {
                    lcd.print(count);
                } else {
                    lcd.print("    "); // Apaga o valor
                }
            }

            char key = ReadKeypad();
            if (key == 'U' && !tarefaIniciada) {
                incrementando = true;
                count++;
                if (count > maxCortes){
                    count = 1; // Retorna a 1 se ultrapassar o máximo
                }
                lcd.setCursor(13, 1);
                lcd.print(count);
                delay(150); // Debounce
                displayCount = true; // Garante que o count seja exibido imediatamente após incremento
            } else if (key == 'D' && !tarefaIniciada) {
                incrementando = true;
                count--;
                if (count < 1){
                    count = maxCortes; // Retorna a maxCortes se for menor que 1
                }
                lcd.setCursor(13, 1);
                lcd.print(count);
                delay(150); // Debounce
                displayCount = true; // Garante que o count seja exibido imediatamente após decremento
            } else {
                incrementando = false;
            }
            
            // Verifica se o botão 'S' foi pressionado por tempoPressionado para iniciar a tarefa
            if (key == 'S' && !tarefaIniciada) {
                unsigned long pressStartMillis = millis();
                while (ReadKeypad() == 'S') {
                    if (millis() - pressStartMillis >= tempoPressionado) {
                        valorDesejado = count; // Define o valor desejado como o valor atual de count
                        iniciarTarefa(menuID, 5);
                        tarefaIniciada = true;
                        botaoConfirmarPressionado = true; // Marca o botão de confirmar como pressionado
                        break;
                    }
                }
            }
            
            // Se o botão de confirmar foi pressionado e a tarefa foi completada, reinicia o menu
            if (botaoConfirmarPressionado && tarefaCompletada()) {
                tarefaIniciada = false; // Reinicia o loop do Menu1
                count = 1; // Reseta count para começar de novo
                valorDesejado = 0; // Reseta o valor desejado
                MainMenuDisplay(); // Retorna ao menu principal
                return; // Sai do loop interno para retornar ao MainMenuDisplay()
            }
        }
    }
}

void Menu3() {  
  int menuID = 3;
    while (true) {
        lcd.setCursor(0, 1);
        lcd.print("Qtde Cortes:");
        lcd.setCursor(13, 1);
        lcd.print(count);

        bool botaoConfirmarPressionado = false; // Flag para verificar se o botão de confirmar foi pressionado

        while (ReadKeypad() != 'L') {
            unsigned long currentMillis = millis();
            
            // Verifica se alcançou o valor desejado e para de piscar
            if (count == valorDesejado && !tarefaIniciada && incrementando) {
                displayCount = true; // Garante que o count seja exibido
                previousMillis = currentMillis; // Reinicia o timer de piscar
            }
            
            // Piscar somente se não estiver incrementando, e se não tiver alcançado o valor desejado
            if (!incrementando && !tarefaIniciada && currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                displayCount = !displayCount;
                lcd.setCursor(13, 1);
                if (displayCount) {
                    lcd.print(count);
                } else {
                    lcd.print("    "); // Apaga o valor
                }
            }

            char key = ReadKeypad();
            if (key == 'U' && !tarefaIniciada) {
                incrementando = true;
                count++;
                if (count > maxCortes){
                    count = 1; // Retorna a 1 se ultrapassar o máximo
                }
                lcd.setCursor(13, 1);
                lcd.print(count);
                delay(150); // Debounce
                displayCount = true; // Garante que o count seja exibido imediatamente após incremento
            } else if (key == 'D' && !tarefaIniciada) {
                incrementando = true;
                count--;
                if (count < 1){
                    count = maxCortes; // Retorna a maxCortes se for menor que 1
                }
                lcd.setCursor(13, 1);
                lcd.print(count);
                delay(150); // Debounce
                displayCount = true; // Garante que o count seja exibido imediatamente após decremento
            } else {
                incrementando = false;
            }
            
            // Verifica se o botão 'S' foi pressionado por tempoPressionado para iniciar a tarefa
            if (key == 'S' && !tarefaIniciada) {
                unsigned long pressStartMillis = millis();
                while (ReadKeypad() == 'S') {
                    if (millis() - pressStartMillis >= tempoPressionado) {
                        valorDesejado = count; // Define o valor desejado como o valor atual de count
                        iniciarTarefa(menuID, 4);
                        tarefaIniciada = true;
                        botaoConfirmarPressionado = true; // Marca o botão de confirmar como pressionado
                        break;
                    }
                }
            }
            
            // Se o botão de confirmar foi pressionado e a tarefa foi completada, reinicia o menu
            if (botaoConfirmarPressionado && tarefaCompletada()) {
                tarefaIniciada = false; // Reinicia o loop do Menu1
                count = 1; // Reseta count para começar de novo
                valorDesejado = 0; // Reseta o valor desejado
                MainMenuDisplay(); // Retorna ao menu principal
                return; // Sai do loop interno para retornar ao MainMenuDisplay()
            }
        }
    }
}

void WaitBtnRelease(){
    while( analogRead(keypad_pin) < 800){} 
}
 
void MainMenuBtn(){
    WaitBtnRelease();
    if(btn_push == 'D')
    {
        mainMenuPage++;
        if(mainMenuPage > mainMenuTotal)
          mainMenuPage = 1;
    }
    else if(btn_push == 'U')
    {
        mainMenuPage--;
        if(mainMenuPage == 0)
          mainMenuPage = mainMenuTotal;    
    }
    
    if(mainMenuPage != mainMenuPageOld) //only update display when page change
    {
        MainMenuDisplay();
        mainMenuPageOld = mainMenuPage;
    }
}
 
void setup(){
    lcd.begin(16,2);  //Initialize a 2x16 type LCD
    MainMenuDisplay();
    delay(1000);

    motor.setSpeed(30);
    S1.attach(SERVO1);
    S2.attach(SERVO2);

    pinMode(SENSOR, INPUT);
}

void loop(){
    btn_push = ReadKeypad();
    
    MainMenuBtn();
    
    if(btn_push == 'S')//enter selected menu
    {
        WaitBtnRelease();
        switch (mainMenuPage)
        {
            case 1:
              Menu1();
              break;
            case 2:
              Menu2();
              break;
            case 3:
              Menu3();
              break;
        }
 
          MainMenuDisplay();
          WaitBtnRelease();
    }
    
 
 
    delay(10);
  
}//--------------- End of loop() loop ---------------------
