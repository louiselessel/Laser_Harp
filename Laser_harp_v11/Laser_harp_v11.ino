/*

   Using Teensy USB MIDI

   You must select MIDI from the "Tools > USB Type" menu
   http://www.pjrc.com/teensy/td_midi.html

   Using a MUX CD4051BE
   And accelerometer Adafruit BNO055

   Code by Louise Lessel and Sid Chou

   Troubleshooting:
   - laser not sending midi? check the ligthLevel, make sure it is the right threshold
*/

#include <Bounce.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);

// Settings ---------------------------------------------------
// Set level of laser light when on
int lightLevel = 400;

// MUX interfacing --------------------------------------------
int readPin = A0;
int inhibit_1 = 11;
int inhibit_2 = 12;


const int outerChannels = 6;
const int innerChannels = 5;
int analogVal = 0;

// Teensy pins for sending address to mux
int addressA = 21;
int addressB = 22;
int addressC = 23;

int A = 0;      //Address pin A
int B = 0;      //Address pin B
int C = 0;      //Address pin C


// MIDI settings ----------------------------------------------
// store previously sent values, to detect changes
int o_pStringOn[outerChannels];
int i_pStringOn[innerChannels];
int accel_pStringOn[3];

elapsedMillis msec = 0;


// Program ----------------------------------------------------
void setup() {
  Serial.begin(9600);
  // Prepare address pins for output
  pinMode(addressA, OUTPUT);
  pinMode(addressB, OUTPUT);
  pinMode(addressC, OUTPUT);
  // Prepare read pin
  pinMode(readPin, INPUT);
  // Prepare chip pins
  pinMode(inhibit_1, OUTPUT);
  pinMode(inhibit_2, OUTPUT);

  // Initialize strings midi messages to 0
  for (int i = 0; i < outerChannels; i++) {
    o_pStringOn[i] = 0;
  }
  for (int i = 0; i < innerChannels; i++) {
    i_pStringOn[i] = 0;
  }

  for (int i = 0; i < 3; i++) {
    accel_pStringOn[i] = 0;
  }


  // Initialize the accelerometer
  Serial.println("Orientation Sensor Test"); Serial.println("");
  /* Initialise the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }
  delay(1000);
  bno.setExtCrystalUse(true);
}

void loop() {

  /* Check the lasers */
  // only check the analog inputs 50 times per second, (20 msec)
  // to prevent a flood of MIDI messages
  if (msec >= 20) {
    msec = 0;
    //Serial.println("MUX 1 - OUTER");
    chooseChip(1);
    checkLasers(1);
    //Serial.println("MUX 2 - INNER");
    chooseChip(2);
    checkLasers(2);

    /* Get a new accelerometer sensor event */
    checkAccelerometer ();
  }


  // DO NOT DELETE THE INCOMING FUNCTION
  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignore incoming messages,
  }
}

void chooseChip(byte chip) {
  // Enable MUX 1
  if (chip == 1) {
    digitalWrite(inhibit_1, LOW);
    digitalWrite(inhibit_2, HIGH);
  }
  // Enable MUX 2
  else if (chip == 2) {
    digitalWrite(inhibit_1, HIGH);
    digitalWrite(inhibit_2, LOW);
  }
}

void checkLasers(byte channel) {
  //Select each pin and read value
  if (channel == 1 ) {
    for (int i = 0; i < outerChannels; i++) {
      A = bitRead(i, 0); //Take first bit from binary value of i channel.
      B = bitRead(i, 1); //Take second bit from binary value of i channel.
      C = bitRead(i, 2); //Take third bit from value of i channel.

      //Write address to mux
      digitalWrite(addressA, A);
      digitalWrite(addressB, B);
      digitalWrite(addressC, C);

      /*
          //Read and print value
          Serial.print("Channel ");
          Serial.print(channel);
          Serial.print(" Num ");
          Serial.print(i);
          Serial.print(" value: ");
      */
      analogVal = analogRead(readPin);
      //Serial.println(analogVal);

      // Note on/off
      // if light is cut off -> note has been struck
      if (analogVal < lightLevel) {
        // if it was off and is now on
        if (o_pStringOn[i] == 0) {
          // send midi in range 1-127 (divide by 8)
          usbMIDI.sendNoteOn(i, analogVal / 8, channel);
          Serial.println("midi ON");
          // store that it has now been turned on
          o_pStringOn[i] = 1;
        }

      }
      // If the light is not cut off - silence the string
      else if (analogVal > lightLevel) {
        // if it was on and is now off
        if (o_pStringOn[i] == 1) {
          usbMIDI.sendNoteOff(i, analogVal / 8, channel);
          Serial.println("midi OFFFFFFFFFFF");
          // store that it is now off
          o_pStringOn[i] = 0;
        }
      }
    }
  }

  if (channel == 2 ) {
    for (int i = 0; i < innerChannels; i++) {
      A = bitRead(i, 0); //Take first bit from binary value of i channel.
      B = bitRead(i, 1); //Take second bit from binary value of i channel.
      C = bitRead(i, 2); //Take third bit from value of i channel.

      //Write address to mux
      digitalWrite(addressA, A);
      digitalWrite(addressB, B);
      digitalWrite(addressC, C);

      /*
          //Read and print value
          Serial.print("Channel ");
          Serial.print(channel);
          Serial.print(" Num ");
          Serial.print(i);
          Serial.print(" value: ");
      */
      analogVal = analogRead(readPin);
      //Serial.println(analogVal);

      // Note on/off
      // if light is cut off -> note has been struck
      if (analogVal < lightLevel) {
        // if it was off and is now on
        if (i_pStringOn[i] == 0) {
          // send midi in range 1-127 (divide by 8)
          usbMIDI.sendNoteOn(i, analogVal / 8, channel);
          Serial.println("midi ON");
          // store that it has now been turned on
          i_pStringOn[i] = 1;
        }

      }
      // If the light is not cut off - silence the string
      else if (analogVal > lightLevel) {
        // if it was on and is now off
        if (i_pStringOn[i] == 1) {
          usbMIDI.sendNoteOff(i, analogVal / 8, channel);
          Serial.println("midi OFFFFFFFFFFF");
          // store that it is now off
          i_pStringOn[i] = 0;
        }
      }
    }
  }
}

void checkAccelerometer () {
  sensors_event_t event;
  bno.getEvent(&event);
  //imu::Vector<3> accel =  bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  //Serial.print("test: ");
  Serial.println(event.orientation.z, 4);

  if (event.orientation.z > 130 && event.orientation.z < 170) {
    Serial.print("NOWWWWWWWWWWWWW");
    // X = 1, Y = 2, Z = 3 , Val, channel 3
    int sendY = map(abs(event.orientation.z), 130, 170, 1, 127);
    Serial.println(sendY);
    usbMIDI.sendNoteOn(2, sendY, 3);
  }
 

  //map(value, fromLow, fromHigh, toLow, toHigh)
  //usbMIDI.sendControlChange(0, map(event.orientation.x, 0, 360, 1, 127), 1);
  //usbMIDI.sendControlChange(0, map(event.orientation.y, 0, 360, 1, 127), 2);
  //usbMIDI.sendControlChange(0, map(event.orientation.z, 0, 360, 1, 127), 3);

  delay(50);
}
