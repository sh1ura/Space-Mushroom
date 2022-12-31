#include "Mouse.h"
#include "Keyboard.h"

#define DELAY 15 // motion sending cycle
#define DEAD_THRES 0 // threshold to ignore small motion
#define SPEED_PARAM 400 // larger is slower

#define DOF 6
#define TX 0 // translation X
#define TY 1 // translation Y
#define TZ 2 // translation Z
#define RX 3 // rotation X
#define RY 4 // rotation Y
#define RZ 5 // rotation Z

// ports of analog input for joysticks
int port[DOF] = {A0, A2, A6, A1, A3, A7};

// conversion matrix from sensor input to rigid motion
int coeff[DOF][DOF] = {
  { 0,  0,  0,-10,-10, 20}, // TX
  { 0,  0,  0,-17, 17,  0}, // TY
  {-3, -3, -3,  0,  0,  0}, // TZ
  {-6,  6,  0,  0,  0,  0}, // RY
  { 3,  3, -6,  0,  0,  0}, // RX
  { 0,  0,  0,  3,  3,  3}, // RZ
};

int origin[DOF]; // initial sensor values

void setup() {
  Mouse.begin();
  Keyboard.begin();
  delay(300);
  for(int i = 0; i < DOF; i++) {
    origin[i] = analogRead(port[i]);
  }
}

int sx, sy, sw;
void move(int x, int y, int w) {
  if(x > DEAD_THRES)
    x -= DEAD_THRES;
  else if(x < -DEAD_THRES)
    x += DEAD_THRES;
  else
    x = 0;

  if(y > DEAD_THRES)
    y -= DEAD_THRES;
  else if(y < -DEAD_THRES)
    y += DEAD_THRES;
  else
    y = 0;

  if(w > DEAD_THRES)
    w -= DEAD_THRES;
  else if(w < -DEAD_THRES)
    w += DEAD_THRES;
  else
    w = 0;

  Mouse.move(x, y, w);

  sx += x;
  sy += y;
  sw += w;
}

// return mouse pointer to the original position
void resetMove(void) {
    Mouse.move(-sx, -sy, -sw);
    sx = 0;
    sy = 0;
    sw = 0;
}

void loop() {
  int sv[DOF]; // sensor value
  int mv[DOF]; // motion vector
  int moveFlag = false;

  for(int i = 0; i < DOF; i++) {
    sv[i] = analogRead(port[i]) - origin[i];
  }

  for(int i = 0; i < DOF; i++) {
    mv[i] = 0;
    for(int j = 0; j < DOF; j++) {
      mv[i] += coeff[i][j] * sv[j];
    }
    mv[i] /= SPEED_PARAM;
    if(mv[i] > 127) {
      mv[i] = 127;
    }
    else if(mv[i] < -128) {
      mv[i] = -128;
    }
  }

  if(abs(mv[RX]) > DEAD_THRES || abs(mv[RY]) > DEAD_THRES) {
    Mouse.press(MOUSE_MIDDLE);
    move(mv[RX], mv[RY], 0);
    Mouse.release(MOUSE_MIDDLE);
  }

  if(abs(mv[TX]) > DEAD_THRES || abs(mv[TY]) > DEAD_THRES) {
    Keyboard.press(KEY_LEFT_SHIFT);
    Mouse.press(MOUSE_MIDDLE);
    move(mv[TX], mv[TY], 0);
    Keyboard.releaseAll();
    Mouse.release(MOUSE_MIDDLE);
  }

  if(abs(mv[TZ]) > DEAD_THRES) {
    Keyboard.press(KEY_LEFT_CTRL);
    Mouse.press(MOUSE_MIDDLE);
    move(0, mv[TZ], 0);
    Keyboard.releaseAll();
    Mouse.release(MOUSE_MIDDLE);
  }

  resetMove();

  delay(DELAY);
}
