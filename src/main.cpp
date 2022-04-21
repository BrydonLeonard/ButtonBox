#include <Arduino.h>
#include <Joystick.h>
#include <shift-register.h>
#include <wrapped-encoder.h>
#include <button/button-manager.h>
#include <Vector.h>

unsigned short TX_LED = 17;
unsigned short RX_LED = 30;

unsigned short PROBE = 4;

// Button matrix
unsigned short BUTTON_ROW_1 = 6;
unsigned short BUTTON_ROW_2 = 7;
unsigned short BUTTON_ROW_3 = 8;
unsigned short BUTTON_ROW_4 = 9;
ShiftRegister *buttonRegister = new ShiftRegister(10, 14, 16);
unsigned short buttonRegisterMaxScanRow = 7;

// Rotary switch
unsigned short ROTARY_READ = 5;
ShiftRegister *rotaryRegister = new ShiftRegister(15, A1, A0);
unsigned short rotaryRegisterMaxScanRow = 8;

// Encoder 1
WrappedEncoder *encoder1 = new WrappedEncoder(0, 1);

// Encoder 2
WrappedEncoder *encoder2 = new WrappedEncoder(2, 3);

// Manages button states
ButtonManager *buttonManager;

Button* createDefaultMomentaryButton(unsigned short pin, unsigned short buttonNumber) {
  ButtonConfig* config = new ButtonConfig {
    Momentary,
    pin,
    buttonNumber,
    25,
    0,
    None
  };
  return new Button(config);
}

Button* createDefaultSyntheticMomentarySwitch(unsigned short pin, unsigned short buttonNumber) {
  ButtonConfig* config = new ButtonConfig {
    SyntheticMomentary,
    pin,
    buttonNumber,
    25,
    30,
    None
  };

  return new Button(config);
}

CompoundButton* createDefaultSyntheticMomentaryWithReleaseSwitch(unsigned short pin, unsigned short buttonNumber, unsigned short syntheticJoystickOutput) {
  ButtonConfig* config = new ButtonConfig {
    SyntheticMomentary,
    pin,
    buttonNumber,
    25,
    30,
    SyntheticPress
  };

  ButtonConfig* innerConfig = new ButtonConfig {
    SyntheticMomentary,
    0, // Not used
    syntheticJoystickOutput,
    25,
    30,
    None
  };

  return new CompoundButton(
    config,
    innerConfig
  );
}
/*

  new unsigned short[29] {
    // Push buttons
    22, 20, 23, 21, 14, 12, 15, 13, 
    // Switches
    6, 4, 8, 10, 19, 17, 1, 3, 
    // Rotary switch
    30, 29, 28, 35, 34, 33, 32, 31, 
    // Encoders
    36, 37, 27, 38, 
    // Encoder pushbutton (only one works currently 🤷)
    39
  },
  new ButtonType[29] {
    // Push buttons
    Momentary, Momentary, Momentary, Momentary, Momentary, Momentary, Momentary, Momentary, 
    // Switches
    // ============================
    // Driving
    // ============================
    SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, 
    SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, 
    // ============================
    // Flying
    // ============================
    //Momentary, Momentary, Momentary, Momentary, Momentary, Momentary, Momentary, Momentary, 
    // Rotary switch
    SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, 
    SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, SyntheticMomentary, 
    // Encoders
    Momentary, Momentary, Momentary, Momentary, 
    // Encoder pushbutton
    Momentary
  },
  40,
  29
  */

// HID interface
Joystick_ joystick(0x03, JOYSTICK_TYPE_JOYSTICK, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

void setup() {
  Serial.begin(9600);
	joystick.begin();
  unsigned short buttonIndex = 0;
  Button** buttons = new Button*[29];

  unsigned short buttonPins[8]  { 22, 20, 23, 21, 14, 12, 15, 13 };
  for (int i = 0; i < 8; i++) {
    buttons[buttonIndex++] = createDefaultMomentaryButton(buttonPins[i], i);
  }

  unsigned short switchPins[8] { 6, 4, 8, 10, 19, 17, 1, 3 };
  for (int i = 0; i < 8; i++) {
    buttons[buttonIndex++] = createDefaultSyntheticMomentarySwitch(switchPins[i], i + 8);
  }

  unsigned short rotarySwitchPoles[8] { 30, 29, 28, 35, 34, 33, 32, 31 };
  for (int i = 0; i < 8; i++) {
    buttons[buttonIndex++] = createDefaultSyntheticMomentarySwitch(rotarySwitchPoles[i], i + 16);
  }

  unsigned short encoderPins[5] { 36, 37, 27, 38, 39 };
  for (int i = 0; i < 5; i++) {
    buttons[buttonIndex++] = createDefaultMomentaryButton(encoderPins[i], i + 24);
  }

  buttonManager = new ButtonManager(buttons, 29, 0);
  Serial.println("Setup complete");

  pinMode(BUTTON_ROW_1, INPUT_PULLUP);
  pinMode(BUTTON_ROW_2, INPUT_PULLUP);
  pinMode(BUTTON_ROW_3, INPUT_PULLUP);
  pinMode(BUTTON_ROW_4, INPUT_PULLUP);

  pinMode(PROBE, INPUT_PULLUP);
  
  pinMode(ROTARY_READ, INPUT_PULLUP);

  buttonRegister->resetHigh();
  rotaryRegister->resetHigh();
}

bool* translateEncoderChangeToStateArray(long& encoderPosition) {
  if (encoderPosition > 0) {
    return new bool[2] { 1, 0 };
  }

  if (encoderPosition < 0) {
    return new bool[2] { 0, 1 };
  }

  return new bool[2] { 1, 1 };
}

int timer = 0;

void scan() {
  // Button matrix + rotary switch + 2 encoders
  unsigned short scanPositions = 40;
  bool states[scanPositions];

  buttonRegister->resetHigh();
  buttonRegister->pushToLatch(0);

  unsigned short statePositionIndex = 0;
  // Scan button matrix
  for (unsigned short col = 0; col < buttonRegisterMaxScanRow; col++) {
    states[statePositionIndex++] = digitalRead(BUTTON_ROW_1);
    states[statePositionIndex++] = digitalRead(BUTTON_ROW_2);
    states[statePositionIndex++] = digitalRead(BUTTON_ROW_3);
    states[statePositionIndex++] = digitalRead(BUTTON_ROW_4);
    buttonRegister->bumpLatch();
  }

  // Scan rotary switch
  rotaryRegister->resetHigh();
  rotaryRegister->pushToLatch(0);
  for (int col = 0; col < 8; col++) {
    states[statePositionIndex++] = digitalRead(ROTARY_READ);
    rotaryRegister->bumpLatch();
  }

  // Scan encoders
  long encoderPosition1 = encoder1->getChange();
  long encoderPosition2 = encoder2->getChange();

  bool* encoder1States = translateEncoderChangeToStateArray(encoderPosition1);
  bool* encoder2States = translateEncoderChangeToStateArray(encoderPosition2);

  states[statePositionIndex++] = *(encoder1States);
  states[statePositionIndex++] = *(encoder1States + 1);
  
  states[statePositionIndex++] = *(encoder2States);
  states[statePositionIndex++] = *(encoder2States + 1);

  buttonManager->updateButtons(states);

  short stateChanges[buttonManager->buttonCount];
  
  buttonManager->getButtonBroadcast(stateChanges);

  bool hasChanges = false;

  for (unsigned long i = 0; i < buttonManager->buttonCount && !hasChanges; i++) {
    int delta = stateChanges[i];
    if (delta != 0) {
      hasChanges = true;
    }
  }

  if (hasChanges) {
    for (unsigned long i = 0; i < buttonManager->buttonCount; i++) {
      if (stateChanges[i] == 1) {
        Serial.print("Released ");
        Serial.println(i);
        joystick.releaseButton(i);
      } else if (stateChanges[i] == -1) {
        Serial.print("Pressed ");
        Serial.println(i);
        joystick.pressButton(i);
      }
    }
  }

  delete encoder1States;
  delete encoder2States;
}

long i = 0;
long encoderVal = 0;

// the loop function runs over and over again forever
void loop() {
  scan();
}
