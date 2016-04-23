// place buttonread and encoder read void here

void buttonRead() {

  for ( int i = 0; i < 4; i++ ) { // the 4 bottom buttons
    byte type = mcpButton.CheckButton(pushList[i]); // current time and length of time to press the button as many times as you can ie. 1.5 seconds
    switch (type)
    {
      case WAITING:
        break;
      case PRESSED:
        viewMode[1] = i;
        Serial.println("subviewmode");
        Serial.println(viewMode[1]);
        updateLed();
        break;
      case DOUBLE_PRESSED:
        Serial.println("viewmode");
        viewMode[0] = i;
        viewMode[1] = 0;
        Serial.println(viewMode[0]);
        updateLed();
        break;
      case MULTI_PRESSED:
        Serial.println("pressed 3 times");
        break;
      case HELD:
        Serial.println("Button HELD");
        break;
    }
  }

  byte type = button.CheckButton(PUSH0); //  Setup button
  switch (type)
  {
    case WAITING:
      break;
    case PRESSED:
      viewMode[1] = 0;
      updateLed();
      Serial.println("back");
      break;
    case DOUBLE_PRESSED:
      Serial.println("pressed 2 times");
      break;
    case MULTI_PRESSED:
      Serial.println("pressed 3 times");
      break;
    case HELD:
      viewMode[0] = SETUP;
      updateLed();
      Serial.println("setup mode");
      break;
  }
}


void encoderRead(int n, int r) { // r= 1 rotated right || r=0 rotated left || n = encoder number
 

  if (r == 1 && controls[viewMode[0]][viewMode[1]][n] < 125) {
    controls[viewMode[0]][viewMode[1]][n] +=3;
  } else if (r == 0 && controls[viewMode[0]][viewMode[1]][n] > 2) {
    controls[viewMode[0]][viewMode[1]][n] -=3;
  }
  Serial.print("enc adjust");
    Serial.print(n);
  Serial.print(viewMode[0]  );
  Serial.print(viewMode[1]  );
  Serial.println(controls[viewMode[0]][viewMode[1]][n]);

}
