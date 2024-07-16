#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Servo.h>

#define STEP 12
#define DIR 13
#define ENABLE 11 
#define SERVO1 3
#define SERVO2 1
#define SENSOR 2

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Servo S1;
Servo S2;

int keypad_pin = A0;
int keypad_value = 0;
int keypad_value_old = 0;
int res_count = 1;
int count = 1;
int maxCortes = 300;
int maxResistor = 50; // Não esquecer de considerar esse valor x10 na função iniciarTarefa()
int valorCorteDesejado = 0;
int valorResistorDesejado = 0;
int resistoresRestantes = 0;

volatile int contador = 0;
 
unsigned long previousMillis = 0;
const unsigned long tempoPressionado = 2000;
const long interval = 500;

bool displayCount = true;
bool incrementando = false;
bool tarefaIniciada = false;
bool confirmResCount = false;
bool confirmCutCount = false;

char btn_push;

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 4;

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
          break;
        case 4:
          lcd.print("4. Outras Qtdes");
          break;
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

void sensorInterrupt() {
  contador++; // Incrementa o contador quando ocorre a interrupção do sensor
}

bool tarefaCompletada() {
    // Implemente a lógica para verificar se a tarefa foi completada
    // Exemplo simples: aguarda um tempo fixo para simular o término da tarefa
    MainMenuDisplay();
    lcd.clear();
    return true; // Retorna verdadeiro para indicar que a tarefa foi completada
}

void iniciarTarefa(int menu, int qtde, int i) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Iniciando tarefa...");
    delay(1000);

    digitalWrite(ENABLE, LOW);
    digitalWrite(DIR, LOW);

    while(i < valorCorteDesejado+1){
        unsigned long previousMicros = micros();
        while (contador < qtde) {
            digitalWrite(STEP, HIGH);
            while (micros() - previousMicros < 350) {}
                digitalWrite(STEP, LOW);
                while (micros() - previousMicros < 350) {}
                    previousMicros += 350;
                }
        digitalWrite(ENABLE, HIGH);

        S1.write(40);
        S2.write(140);
        delay(200);
        S1.write(180);
        S2.write(0);
        i++;
    }

     digitalWrite(ENABLE, HIGH);

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Tarefa concluida!");
    tarefaCompletada();
    resistoresRestantes -= 15;
}

void Menu1() {
    const int menuID = 1;  
    while (true) {
        lcd.setCursor(0, 1);
        lcd.print("Qtde Cortes:");
        lcd.setCursor(13, 1);
        lcd.print(count);

        bool botaoConfirmarPressionado = false;

        while (ReadKeypad() != 'L') {
            unsigned long currentMillis = millis();
            
            // Verifica se alcançou o valor desejado e para de piscar
            if (count == valorCorteDesejado && !tarefaIniciada && incrementando) {
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
                        valorCorteDesejado = count; // Define o valor desejado como o valor atual de count
                        if (resistoresRestantes == 0) {
                            iniciarTarefa(menuID, 18, count); // Inicia com 17 resistores se há restantes
                            resistoresRestantes = 3; // Reduz os restantes em 2
                        } else {
                            iniciarTarefa(menuID, 15, count); // Inicia com 15 resistores
                        } // Passa o menuID para iniciarTarefa
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
                valorCorteDesejado = 0; // Reseta o valor desejado
                MainMenuDisplay(); // Retorna ao menu principal
                return; // Sai do loop interno para retornar ao MainMenuDisplay()
            }
        }
    }
}

void Menu2() {
    const int menuID = 2;  
    while (true) {
        lcd.setCursor(0, 1);
        lcd.print("Qtde Cortes:");
        lcd.setCursor(13, 1);
        lcd.print(count);

        bool botaoConfirmarPressionado = false; // Flag para verificar se o botão de confirmar foi pressionado

        while (ReadKeypad() != 'L') {
            unsigned long currentMillis = millis();
            
            // Verifica se alcançou o valor desejado e para de piscar
            if (count == valorCorteDesejado && !tarefaIniciada && incrementando) {
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
                        valorCorteDesejado = count; // Define o valor desejado como o valor atual de count
                        iniciarTarefa(menuID, 5, count);
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
                valorCorteDesejado = 0; // Reseta o valor desejado
                MainMenuDisplay(); // Retorna ao menu principal
                return; // Sai do loop interno para retornar ao MainMenuDisplay()
            }
        }
    }
}

void Menu3() {
    const int menuID = 3;
    bool botaoConfirmarPressionado = false; // Flag para verificar se o botão de confirmar foi pressionado

    while (true) {
        lcd.setCursor(0, 1);
        lcd.print("Qtde Cortes:");
        lcd.setCursor(13, 1);
        lcd.print(count);

        while (ReadKeypad() != 'L') {
            unsigned long currentMillis = millis();
            
            // Verifica se alcançou o valor desejado e para de piscar
            if (count == valorCorteDesejado && !tarefaIniciada && incrementando) {
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
                        valorCorteDesejado = count; // Define o valor desejado como o valor atual de count
                        iniciarTarefa(menuID, 4, count);
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
                valorCorteDesejado = 0; // Reseta o valor desejado
                MainMenuDisplay(); // Retorna ao menu principal
                return; // Sai do loop interno para retornar ao MainMenuDisplay()
            }
        }
    }
}

void Menu4(){
    const int menuID = 4;
    if(confirmResCount == false){
        while (true) {
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Qtde (x10):");
            lcd.setCursor(12, 0);
            lcd.print(res_count);

            while (ReadKeypad() != 'L') {
                unsigned long currentMillis = millis();
                
                // Verifica se alcançou o valor desejado e para de piscar
                if (count == valorCorteDesejado && !tarefaIniciada && incrementando) {
                    displayCount = true; // Garante que o count seja exibido
                    previousMillis = currentMillis; // Reinicia o timer de piscar
                }
                
                // Piscar somente se não estiver incrementando, e se não tiver alcançado o valor desejado
                if (!incrementando && !tarefaIniciada && currentMillis - previousMillis >= interval) {
                    previousMillis = currentMillis;
                    displayCount = !displayCount;
                    lcd.setCursor(12, 0);
                    if (displayCount) {
                        lcd.print(res_count);
                    } else {
                        lcd.print("    "); // Apaga o valor
                    }
                }

                char key = ReadKeypad();
                if (key == 'U' && !tarefaIniciada) {
                    incrementando = true;
                    res_count++;
                    if (res_count > maxResistor){
                        res_count = 1; // Retorna a 1 se ultrapassar o máximo
                    }
                    lcd.setCursor(12, 0);
                    lcd.print(res_count);
                    delay(150); // Debounce
                    displayCount = true; // Garante que o count seja exibido imediatamente após incremento
                } else if (key == 'D' && !tarefaIniciada) {
                    incrementando = true;
                    res_count--;
                    if (res_count < 1){
                        res_count = maxResistor; // Retorna a maxCortes se for menor que 1
                    }
                    lcd.setCursor(12, 0);
                    lcd.print(res_count);
                    delay(150); // Debounce
                    displayCount = true; // Garante que o count seja exibido imediatamente após decremento
                } else {
                    incrementando = false;
                }
                
                if (key == 'S') {
                    valorResistorDesejado = res_count; // Define o valor desejado como o valor atual de count
                    while (true) {
                        lcd.setCursor(1, 1);
                        lcd.print("Qtde Cortes:");
                        lcd.setCursor(13, 1);
                        lcd.print(count);

                        bool botaoConfirmarPressionado = false; // Flag para verificar se o botão de confirmar foi pressionado

                        while (ReadKeypad() != 'L') {
                            unsigned long currentMillis = millis();
                            
                            // Verifica se alcançou o valor desejado e para de piscar
                            if (count == valorCorteDesejado && !tarefaIniciada && incrementando) {
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
                                        valorCorteDesejado = count; // Define o valor desejado como o valor atual de count
                                        iniciarTarefa(menuID, 4, count);
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
                                valorCorteDesejado = 0; // Reseta o valor desejado
                                valorResistorDesejado = 0;
                                MainMenuDisplay(); // Retorna ao menu principal
                                return; // Sai do loop interno para retornar ao MainMenuDisplay()
                            }
                        }  
                    }
                }
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

    pinMode(STEP, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(ENABLE, OUTPUT);
    pinMode(SENSOR, INPUT);

    S1.attach(SERVO1);
    S2.attach(SERVO2);

    S1.write(180);
    S2.write(0);

    attachInterrupt(digitalPinToInterrupt(SENSOR), sensorInterrupt, RISING);
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
            case 4:
              Menu4();
              break;
        }
 
          MainMenuDisplay();
          WaitBtnRelease();
    }
    delay(10);
}//--------------- End of loop() loop ---------------------
