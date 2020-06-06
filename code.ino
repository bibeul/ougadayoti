#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>


// Variables
// Piezos
#define NB_PIEZO 2
int delays[NB_PIEZO] = {500, 500};
int tonePin[NB_PIEZO] = {13, 12};
int toneValue[NB_PIEZO] = {880, 988};

int tonePos = 0;
int previousMsPiezo = 0;

bool piezoStatus = false;


// Distance Sensors
#define NB_DISTANCE_SENSOR 3
#define THRESHOLD 5.0
int sensorPin[NB_DISTANCE_SENSOR] = {11, 10, 9};


// NeoPixel Leds
#define PIN_NEO 8
#define NUM_LED 8
#define PATTERN_LENGTH 9
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LED, PIN_NEO, NEO_GRB + NEO_KHZ800);
uint32_t SOS[PATTERN_LENGTH] = {200, 200, 200, 600, 600, 600, 200, 200, 200};
uint32_t color = pixels.Color(255, 0, 0);

int patternPos = 0;
bool ledsStatus = true;
int previousMsLed = 0;


// LCD Screen
#define priceAmount 30
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);


// Shared Var
bool systemStatus = true;

String IBAN = "";


// TOOLS
void print(const char *name, int val)
{
    Serial.write(name);
    Serial.write(String(val).c_str());
    Serial.write("\n");
}


// Functions
// Piezos
void loopPiezos()
{
    int currentMillis = millis();

    if (currentMillis - previousMsPiezo >= delays[tonePos])
    {
        //print("Pin: ", tonePin[tonePos]);
        previousMsPiezo = currentMillis;

        int prevTone = (tonePos == 0) ? tonePin[NB_PIEZO - 1] : tonePin[tonePos - 1];
        //print("PrevPin: ", prevTone);

        noTone(prevTone);
        tone(tonePin[tonePos], toneValue[tonePos], delays[tonePos]);

        tonePos = (tonePos == NB_PIEZO - 1) ? 0 : tonePos + 1;
    }
}


// Distance Sensor
long readUltrasonicDistance(int triggerPin, int echoPin)
{
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);

    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pinMode(echoPin, INPUT);
    return pulseIn(echoPin, HIGH);
}

bool checkDistance()
{
    double distance = THRESHOLD + 1;
    for (uint32_t cnt = 0; cnt < NB_DISTANCE_SENSOR; cnt++)
    {
        double tmp = 0.01723 * (double) readUltrasonicDistance(sensorPin[cnt], sensorPin[cnt]);
        if (tmp < distance)
        {
            distance = tmp;
        }
    }
    // Serial.print(distance);
    // Serial.println(" cm\n");
    return (THRESHOLD >= distance) ? true : false;
}


// NeoPixel Leds
void loopNeoPixel()
{
    int currentMillis = millis();

    if (currentMillis - previousMsLed >= SOS[patternPos])
    {
        previousMsLed = currentMillis;

        if (ledsStatus)
        {
            pixels.fill(color, 0, NUM_LED);
        }
        else
        {
            pixels.clear();

            if (patternPos == PATTERN_LENGTH - 1) patternPos = 0;
            else patternPos++;
        }

        ledsStatus = !ledsStatus;
        pixels.show();
    }
}


// ESP setup simulation
bool setupESP()
{
    return true;
}

String getInputMsgFromESP()
{
    if (Serial.available() > 0)
    {
        // read the incoming byte:
        String incomingMsg = Serial.readString();

        // say what you got:
        Serial.print("Received: ");
        Serial.println(incomingMsg);

        return incomingMsg;
    }

    return "";
}


// LCD
void clearRow1()
{
    lcd.setCursor(0, 1);
    lcd.print("               ");
    lcd.setCursor(0, 1);
}

void rfidPayementSimulation()
{
    clearRow1();
    lcd.setCursor(0, 1);
    if (IBAN == "")
    {
        lcd.print("    No IBAN");
    }
    else
    {
        lcd.print("  You gain ");
        lcd.print(priceAmount);
        lcd.print("$");
    }
}


void handleMsg(String msg)
{
    if (msg == "") return;

    lcd.setCursor(0, 0);
    lcd.print("   Anti-Zouk");
    clearRow1();
    lcd.setCursor(0, 1);

    if (msg == "ACTIVATE_PIEZO")
    {
        lcd.print(">> Sound ON <<");
        piezoStatus = true;
    }
    else if (msg == "DEACTIVATE_PIEZO")
    {
        lcd.print(">> Sound OFF <<");
        piezoStatus = false;
    }
    else if (msg == "ACTIVATE_ANTI-ZOUK")
    {
        lcd.print(">> Activated <<");
        systemStatus = true;
    }
    else if (msg == "DEACTIVATE_ANTI-ZOUK")
    {
        lcd.print("! Deactivated !");
        systemStatus = false;
    }
    else if (msg.substring(0, 5) == "IBAN=")
    {
        IBAN = msg.substring(5);
        lcd.print("   IBAN Set   ");
    }
}


void setup()
{
    Serial.begin(9600);
    for (uint32_t cnt = 0; cnt < NB_PIEZO; cnt++)
        pinMode(tonePin[cnt], OUTPUT);

    pixels.begin();
    pixels.show();

    lcd.clear();
    lcd.begin(16, 2);

    if (setupESP()) Serial.println("ESP Setup: OK");

    handleMsg("ACTIVATE_ANTI-ZOUK");
}

void loop()
{
    bool zoukerClose = checkDistance();
    loopNeoPixel();

    if (piezoStatus) loopPiezos();
    Serial.print("systemStatus ");
    Serial.println(systemStatus);
    Serial.print("zoukerClose ");
    Serial.println(zoukerClose);
    if (systemStatus && zoukerClose) rfidPayementSimulation();

    handleMsg(getInputMsgFromESP());

    delay(40);
}
