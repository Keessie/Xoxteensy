// show stuff on OLED screen and light LEDS


void updateLed() {
  for (int i = 0; i < 4; i++) {
    mcp.digitalWrite(ledList[i], LOW);
  }
  digitalWrite(LED0, LOW);

  if (viewMode[0]  < SETUP ) {
    mcp.digitalWrite(ledList[viewMode[0]], HIGH);
  } else {
    digitalWrite(LED0, HIGH);
  }
}


void updateOLED() {

  u8g.firstPage();   // DRAW on screen
  do {
    drawBox();
    //drawLFO();
  } while ( u8g.nextPage());

}


void drawBox(void) {
  u8g.setFont(u8g_font_5x8);

  if (viewMode[0] < SETUP) {

    int value[4];

    for (int i = 0; i < 4; i++) {
      value[i] = controls[viewMode[0]][viewMode[1]][i] / 4;
      u8g.setColorIndex(3);// create brightest square

    }
    u8g.drawStr(0, 10, controlsRot[(viewMode[0] * 4) + viewMode[1]][0]); // draw page name

    // draw names under parameters
    u8g.drawStr(20, 10, controlsRot[(viewMode[0] * 4) + viewMode[1]][1]);
    u8g.drawStr(76, 10,  controlsRot[(viewMode[0] * 4) + viewMode[1]][2]);
    u8g.drawStr(140, 10,  controlsRot[(viewMode[0] * 4) + viewMode[1]][3]);
    u8g.drawStr(195, 10,  controlsRot[(viewMode[0] * 4) + viewMode[1]][4]);

    if ((viewMode[1] == 2) || (viewMode[0] == 2 && viewMode[1] == 1)) { // ADSR Mode
      int xleftCorner = 150;
      int yleftCorner = 20;
      int yBottom = 50;
      int xsustainLength = 30;
      u8g.drawLine(xleftCorner, yleftCorner, xleftCorner - (value[0]+1), yBottom); // attack line
      u8g.drawLine(xleftCorner, yleftCorner, xleftCorner + (value[1]+1), yBottom - value[2]); //decay line
      u8g.drawLine(xleftCorner + value[1]+1, yBottom - value[2], xleftCorner + (value[1]+1) + xsustainLength, yBottom - value[2]   ); //sustain line
      u8g.drawLine(xleftCorner + value[1]+1 + xsustainLength, yBottom - value[2], xleftCorner + (value[1]+1) + xsustainLength + value[3], yBottom); // release line
    }
    if ((viewMode[1] == 3) || (viewMode[0] == 1 && viewMode[1] == 1)) { // LFO Mode
      int xLFOcenter = 120;
      int yLFObottom = 50;
      for (int i = 0; i < 257; i++) {
        if (controls[viewMode[0]][viewMode[1]][2] < 10) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(a + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(a + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
          //u8g.drawPixel(xLFOcenter + (i / (3+controls[viewMode[0]][viewMode[1]][0] / 5)), yLFObottom - ((waveTablepointer[1][i]))/(8+(127-controls[viewMode[0]][viewMode[1]][1])));
        }
        if ((controls[viewMode[0]][viewMode[1]][2] >= 10) && (controls[viewMode[0]][viewMode[1]][2] < 20)) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(b + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(b + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
        }
        if ((controls[viewMode[0]][viewMode[1]][2] >= 20) && (controls[viewMode[0]][viewMode[1]][2] < 30)) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(c + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(c + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
        }
        if ((controls[viewMode[0]][viewMode[1]][2] >= 30) && (controls[viewMode[0]][viewMode[1]][2] < 40)) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(d + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(d + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
        }
        if ((controls[viewMode[0]][viewMode[1]][2] >= 40) && (controls[viewMode[0]][viewMode[1]][2] < 50)) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(e + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(e + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
        }
        if ((controls[viewMode[0]][viewMode[1]][2] >= 50) && (controls[viewMode[0]][viewMode[1]][2] < 60)) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(f + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(f + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
        }
        if ((controls[viewMode[0]][viewMode[1]][2] >= 60) && (controls[viewMode[0]][viewMode[1]][2] < 127)) {
          u8g.drawPixel(xLFOcenter + (i / (3 + controls[viewMode[0]][viewMode[1]][0] / 5)), 25 - ((( *(g + i) / ((127 - controls[viewMode[0]][viewMode[1]][1]))) / 2) - ( *(g + i) / ((127 - controls[viewMode[0]][viewMode[1]][1])))));
        }
      }
    } else if(viewMode[1] ==0) {

      // draw values in square boxes
      for (int i = 0; i < 4; i++) {
        u8g.drawBox((i * 64) + 22, 53 - (value[i]), 20, value[i] + 1);  // draw 3rd brightest square

        u8g.setColorIndex(2); // create 2nd brightest square
        u8g.drawBox((i * 64) + 22, 51 - (value[i]), 20, 2);

        u8g.setColorIndex(1);  // create 1st brightest square
        u8g.drawBox((i * 64) + 23, 49 - (value[i]), 18, 2);
      }
    }
           u8g.setColorIndex(1);  // create 1st brightest square

    // draw buttonnames
    u8g.drawStr(20, 64, controlsBut[viewMode[0]][0]);
    u8g.drawStr(76, 64,  controlsBut[viewMode[0]][1]);
    u8g.drawStr(140, 64,  controlsBut[viewMode[0]][2]);
    u8g.drawStr(195, 64, controlsBut[viewMode[0]][3]);


  }
}


