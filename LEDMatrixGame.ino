// Tetris / Snake / Plane 

#include <LedControl.h>

// 7-segment LED Display
const int DIG1=A3;
const int DIG2=A4;
const int DIG3=A5;

const int SEG_A=5;
const int SEG_B=6;
const int SEG_C=7;
const int SEG_D=8;
const int SEG_E=9;
const int SEG_F=12;
const int SEG_G=11;

const int segments[7]={SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G};

// Common Cathode Display
const byte displayNumbers[10][7]={
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};


// Action Button 
const int actionButton=A0;


// Mode Button
const int modeButton=13;


// Passive Buzzer
const int buzzer=10;


// Joystick
const int joystick_X=A1;
const int joystick_Y=A2;

typedef struct Joystick {
  int X;
  int Y;
} Joystick;

const int JOYSTICK_MIN=150;
const int JOYSTICK_MAX=850;


// LED Matrix MAX7219
const int LED_CS=2;
const int LED_CLK=3;
const int LED_DIN=4;

LedControl ledMatrix(LED_DIN, LED_CLK, LED_CS, 4);

// Game
int score=0; // Max: 999

enum Mode {
  TETRIS,
  SNAKE,
  PLANE
};

Mode currentMode=TETRIS;
int modeCounter=0;
bool paused=false;


// Snake
const int SNAKE_MAX_LENGTH=256; // 8*32

struct Point {
  int x;
  int y;
};

Point snakeBody[SNAKE_MAX_LENGTH];
Point snakeFood;

int snakeLength=3;

enum SnakeDirection {
  SNAKE_UP,
  SNAKE_DOWN,
  SNAKE_LEFT,
  SNAKE_RIGHT
};

SnakeDirection snakeDirection=SNAKE_RIGHT;
SnakeDirection snakeNextDirection=SNAKE_RIGHT;

unsigned long snakeLastMove=0;
unsigned long snakeMoveInterval=250;
bool snakeGameOver=false;
bool snakeStarted=false;
bool snakeFlashState=false;
unsigned long snakeLastFlash=0;

// Plane
const int PLANE_START_X=3;
const int PLANE_START_Y=28;

const int PLANE_MIN_X=2;
const int PLANE_MAX_X=5;

const int PLANE_MIN_Y=25;
const int PLANE_MAX_Y=30;

const int MAX_ENEMIES=5;
const int MAX_BULLETS=3;

struct PlaneEnemy {
  int x;
  int y;
  bool active;
};

struct Bullet {
  int x;
  int y;
  bool active;
};

int planeX=PLANE_START_X;
int planeY=PLANE_START_Y;

PlaneEnemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];

bool planeStarted=false;
bool planeGameOver=false;
bool planeFlashState=false;

unsigned long planeLastMove=0;
unsigned long planeMoveInterval=100;

unsigned long enemyLastMove=0;
unsigned long enemyMoveInterval=300;

unsigned long bulletLastMove=0;
unsigned long bulletMoveInterval=60;

unsigned long planeLastFlash=0;

// Tetris

const int TETRIS_WIDTH=8;
const int TETRIS_HEIGHT=32;

bool tetrisBoard[TETRIS_WIDTH][TETRIS_HEIGHT];

struct TetrisPiece {
  byte type;
  byte rotation;
  int x;
  int y;
};

TetrisPiece currentPiece;

const byte tetrisPieces[7][4][4]={
  {
    {0,0,0,0},
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0}
  },

  {
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  {
    {0,1,0,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  {
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  {
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  {
    {1,0,0,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  {
    {0,0,1,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  }
};

unsigned long tetrisLastMove=0;
unsigned long tetrisMoveInterval=600;
bool tetrisStarted=false;

bool tetrisGameOver=false;
bool tetrisFlashState=false;
unsigned long tetrisLastFlash=0;

bool tetrisMoveLeftHeld=false;
bool tetrisMoveRightHeld=false;

// LED Matrix -------------------------------------------

void setPixel(int x, int y, bool state) {
  if (x<0 || x>7 || y<0 || y>31) {
    return;
  }

  int module=y/8; // which of the four matrices
  int localY=y%8;

  int row=x;
  int column=7-localY; // Y is inverted

  ledMatrix.setLed(module, row, column, state);
}

void clearMatrix() {
  for (int i=0; i<4; i++) {
    ledMatrix.clearDisplay(i);
  }
}

void setupMatrix() {
  for (int i=0; i<4; i++) {
    ledMatrix.shutdown(i, false);
    ledMatrix.setIntensity(i, 3);
    ledMatrix.clearDisplay(i);
  }
}

// Display ----------------------------------------------------

void allDigitsOff() {
  digitalWrite(DIG1, LOW);
  digitalWrite(DIG2, LOW);
  digitalWrite(DIG3, LOW);
}

void displayDigit(int number, int digitPin) {
  allDigitsOff();

  for (int i=0; i<7; i++) {
    digitalWrite(segments[i], displayNumbers[number][i]);
  }

  digitalWrite(digitPin, HIGH);
}

void refreshDisplay() { 
  // only one digit is active at a time

  int firstDigit=score/100;
  int secondDigit=(score/10)%10;
  int thirdDigit=score%10;

  static byte digit=0;
  static unsigned long lastRefresh=0;

  if (micros()-lastRefresh<2000) {
    return;
  }

  lastRefresh=micros();

  int number;
  int digitPin;

  switch (digit) {
    case 0: 
      number=firstDigit;
      digitPin=DIG1;
      break;
    
    case 1: 
      number=secondDigit;
      digitPin=DIG2;
      break;

    default: 
      number=thirdDigit;
      digitPin=DIG3;
      break;
  }

  displayDigit(number, digitPin);

  digit++;
  if (digit>=3) digit=0;
}

// Buzzer ---------------------------------------------

void beep() {
  tone(buzzer, 1000, 100);
}

// Buttons --------------------------------------------

bool readActionButton() {
  static bool lastReading=HIGH;
  static bool stableState=HIGH;
  static unsigned long lastDebounceTime=0;

  bool currentReading=digitalRead(actionButton); 

  if (currentReading!=lastReading) {
    lastDebounceTime=millis();
  }

  if (millis()-lastDebounceTime>30) { // wait for debounce
    if (currentReading!=stableState) {
      stableState=currentReading;

      if (stableState==LOW) { // button was pressed
        Serial.println("action button was pressed");
        lastReading=currentReading;
        return true;
      }
    }
  }

  lastReading=currentReading;
  return false;
}

void readModeButton() {
  static bool lastReading=HIGH;
  static bool stableState=HIGH;
  static unsigned long lastDebounceTime=0;
  static unsigned long startPressTime=0;
  static bool longPressHandled=false;

  bool currentReading=digitalRead(modeButton); 

  if (currentReading!=lastReading) {
    lastDebounceTime=millis();
  }

  if (millis()-lastDebounceTime>30) { // wait for debounce
    if (currentReading!=stableState) {
      stableState=currentReading;

      if (stableState==LOW) { // button was just pressed 
        lastReading=currentReading;
        startPressTime=millis();
        longPressHandled=false;
      }

      else { // button was just released
        if(!longPressHandled) {
          paused=!paused; // it wasn't a long press
          Serial.println("mode button was pressed shortly");
        }
      }
    }

    // button is being held
    if (stableState==LOW && !longPressHandled) {
      if (millis()-startPressTime>=2000) { // was held for 2 seconds
        Serial.println("mode button was pressed for 2 seconds");
        longPressHandled=true;
        modeCounter++;

        if (modeCounter>2) modeCounter=0;
        currentMode=(Mode) modeCounter;
        clearMatrix();

        if (currentMode==TETRIS) {
          tetrisStarted=false;
        }

        if (currentMode==SNAKE) {
          snakeStarted=false;
        }

        if (currentMode==PLANE) {
          planeStarted=false;
        }

        paused=false;
        beep();
      }
    }
  }

  lastReading=currentReading;
}

// Joystick -------------------------------------

Joystick readJoystick() {
  Joystick js;

  js.X=analogRead(joystick_X);
  js.Y=analogRead(joystick_Y);

  return js;
}

bool joystickUp(Joystick js) {
  return js.X<JOYSTICK_MIN;
}

bool joystickDown(Joystick js) {
  return js.X>JOYSTICK_MAX;
}

bool joystickLeft(Joystick js) {
  return js.Y>JOYSTICK_MAX;
}

bool joystickRight(Joystick js) {
  return js.Y<JOYSTICK_MIN;
}

// Tetris ---------------------------------

void clearTetrisBoard() {
  for (int x=0; x<TETRIS_WIDTH; x++) {
    for (int y=0; y<TETRIS_HEIGHT; y++) {
      tetrisBoard[x][y]=false;
    }
  }
}

bool tetrisPieceCell(byte type, byte rotation, int px, int py) {
  if (px<0 || px>=4 || py<0 || py>=4) {
    return false;
  }

  int x=px;
  int y=py;

  for (byte i=0; i<rotation; i++) {
    int newX=3-y;
    int newY=x;

    x=newX;
    y=newY;
  }

  return tetrisPieces[type][y][x];
}

bool tetrisCollision(int newX, int newY, byte newRotation) {
  for (int py=0; py<4; py++) {
    for (int px=0; px<4; px++) {

      if (!tetrisPieceCell(currentPiece.type, newRotation, px, py)) {
        continue;
      }

      int boardX=newX+px;
      int boardY=newY+py;

      if (boardX<0 || boardX>=TETRIS_WIDTH) {
        return true;
      }

      if (boardY>=TETRIS_HEIGHT) {
        return true;
      }

      if (boardY>=0 && tetrisBoard[boardX][boardY]) {
        return true;
      }
    }
  }

  return false;
}

void drawTetrisPiece() {
  for (int py=0; py<4; py++) {
    for (int px=0; px<4; px++) {

      if (tetrisPieceCell(currentPiece.type, currentPiece.rotation, px, py)) {
        setPixel(currentPiece.x+px, currentPiece.y+py, true);
      }
    }
  }
}

void drawTetrisBoard() {
  clearMatrix();

  for (int x=0; x<TETRIS_WIDTH; x++) {
    for (int y=0; y<TETRIS_HEIGHT; y++) {
      if (tetrisBoard[x][y]) {
        setPixel(x, y, true);
      }
    }
  }

  if (!tetrisGameOver) {
    drawTetrisPiece();
  }
}

void spawnTetrisPiece() {
  currentPiece.type=random(0, 7);
  currentPiece.rotation=0;

  currentPiece.x=2;
  currentPiece.y=0;

  if (tetrisCollision(currentPiece.x, currentPiece.y, currentPiece.rotation)) {
    tetrisGameOver=true;
    tetrisLastFlash=millis();
    tetrisFlashState=false;
    clearMatrix();
    tone(buzzer, 200, 300);
  }
}

void lockTetrisPiece() {
  for (int py=0; py<4; py++) {
    for (int px=0; px<4; px++) {

      if (tetrisPieceCell(currentPiece.type, currentPiece.rotation, px, py)) {
        int boardX=currentPiece.x+px;
        int boardY=currentPiece.y+py;

        if (boardX>=0 && boardX<TETRIS_WIDTH &&
            boardY>=0 && boardY<TETRIS_HEIGHT) {
          tetrisBoard[boardX][boardY]=true;
        }
      }
    }
  }

  tone(buzzer, 500, 40);
}

void clearTetrisLines() {
  int linesCleared=0;

  for (int y=TETRIS_HEIGHT-1; y>=0; y--) {

    bool full=true;

    for (int x=0; x<TETRIS_WIDTH; x++) {
      if (!tetrisBoard[x][y]) {
        full=false;
        break;
      }
    }

    if (full) {
      linesCleared++;

      for (int moveY=y; moveY>0; moveY--) {
        for (int x=0; x<TETRIS_WIDTH; x++) {
          tetrisBoard[x][moveY]=tetrisBoard[x][moveY-1];
        }
      }

      for (int x=0; x<TETRIS_WIDTH; x++) {
        tetrisBoard[x][0]=false;
      }

      y++;
    }
  }

  if (linesCleared>0) {
    score+=linesCleared*10;

    if (score>999) {
      score=999;
    }

    tone(buzzer, 1800, 100);
  }
}

void moveTetrisDown() {
  if (!tetrisCollision(currentPiece.x, currentPiece.y+1, currentPiece.rotation)) {
    currentPiece.y++;
  }

  else {
    lockTetrisPiece();
    clearTetrisLines();
    spawnTetrisPiece();
  }
}

void moveTetrisLeft() {
  if (!tetrisCollision(currentPiece.x-1, currentPiece.y, currentPiece.rotation)) {
    currentPiece.x--;
  }
}

void moveTetrisRight() {
  if (!tetrisCollision(currentPiece.x+1, currentPiece.y, currentPiece.rotation)) {
    currentPiece.x++;
  }
}

void rotate() {
  if (currentMode!=TETRIS || !tetrisStarted || tetrisGameOver || paused) {
    return;
  }

  byte newRotation=currentPiece.rotation+1;

  if (newRotation>=4) {
    newRotation=0;
  }

  if (!tetrisCollision(currentPiece.x, currentPiece.y, newRotation)) {
    currentPiece.rotation=newRotation;
    tone(buzzer, 900, 40);
    return;
  }

  // try moving the piece slightly if rotation is blocked
  if (!tetrisCollision(currentPiece.x-1, currentPiece.y, newRotation)) {
    currentPiece.x--;
    currentPiece.rotation=newRotation;
    tone(buzzer, 900, 40);
    return;
  }

  if (!tetrisCollision(currentPiece.x+1, currentPiece.y, newRotation)) {
    currentPiece.x++;
    currentPiece.rotation=newRotation;
    tone(buzzer, 900, 40);
    return;
  }
}

void startTetris() {
  clearTetrisBoard();

  score=0;

  tetrisGameOver=false;
  tetrisFlashState=false;

  tetrisStarted=true;

  tetrisLastMove=millis();
  tetrisMoveInterval=600;

  tetrisLastFlash=millis();

  spawnTetrisPiece();
  drawTetrisBoard();
}

void updateTetrisGameOver() {
  if (!tetrisGameOver) {
    return;
  }

  if (millis()-tetrisLastFlash>=250) {
    tetrisLastFlash=millis();
    tetrisFlashState=!tetrisFlashState;

    if (tetrisFlashState) {
      for (int y=0; y<32; y++) {
        for (int x=0; x<8; x++) {
          setPixel(x, y, true);
        }
      }
    }

    else {
      clearMatrix();
    }
  }
}

void updateTetrisMovement() {
  Joystick js=readJoystick();

  if (joystickLeft(js)) {
    if (!tetrisMoveLeftHeld) {
      moveTetrisLeft();
      tetrisMoveLeftHeld=true;
    }
  }

  else if (joystickRight(js)) {
    if (!tetrisMoveRightHeld) {
      moveTetrisRight();
      tetrisMoveRightHeld=true;
    }
  }

  else {
    tetrisMoveLeftHeld=false;
    tetrisMoveRightHeld=false;
  }

  if (joystickDown(js)) {
    moveTetrisDown();
    tetrisLastMove=millis();
  }
}

void tetris() {
  if (!tetrisStarted) {
    startTetris();
  }

  if (tetrisGameOver) {
    updateTetrisGameOver();
    return;
  }

  if (paused) {
    return;
  }

  updateTetrisMovement();

  if (millis()-tetrisLastMove>=tetrisMoveInterval) {
    tetrisLastMove=millis();
    moveTetrisDown();
  }

  drawTetrisBoard();
}

// Snake --------------------------------------

void startSnake() {
  snakeLength=3;

  snakeBody[0].x=3; // head
  snakeBody[0].y=15;

  snakeBody[1].x=2;
  snakeBody[1].y = 15;

  snakeBody[2].x=1; // tail
  snakeBody[2].y=15;

  snakeDirection=SNAKE_RIGHT;
  snakeNextDirection=SNAKE_RIGHT;
  snakeMoveInterval=250;

  score=0;
  snakeGameOver=false;
  snakeStarted=true;

  snakeLastMove=millis();
  snakeFlashState=false;
  snakeLastFlash=millis();

  spawnSnakeFood();
  drawSnake();
}

bool snakeAte(int x, int y) {
  for (int i=0; i<snakeLength; i++) {
    if (snakeBody[i].x==x && snakeBody[i].y==y) {
      return true;
    }
  }

  return false;
}


void spawnSnakeFood() {
  do {
    snakeFood.x=random(0, 8);
    snakeFood.y=random(0, 32);
  } while (snakeAte(snakeFood.x, snakeFood.y));
}

void drawSnake() {
  clearMatrix();

  setPixel(snakeFood.x, snakeFood.y, true); // food

  for (int i=0; i<snakeLength; i++) { // snake
    setPixel(snakeBody[i].x, snakeBody[i].y, true);
  }
}

void updateSnakeDirection() {
  Joystick js=readJoystick();

  if (joystickUp(js)) {
    if (snakeNextDirection!=SNAKE_DOWN) {
      snakeNextDirection=SNAKE_UP;
    }
  }

  else if (joystickDown(js)) {
    if (snakeNextDirection!=SNAKE_UP) {
      snakeNextDirection=SNAKE_DOWN;
    }
  }

  else if (joystickLeft(js)) {
    if (snakeNextDirection!=SNAKE_RIGHT) {
      snakeNextDirection=SNAKE_LEFT;
    }
  }

  else if (joystickRight(js)) {
    if (snakeNextDirection!=SNAKE_LEFT) {
      snakeNextDirection=SNAKE_RIGHT;
    }
  }
}

void moveSnake() {
  snakeDirection=snakeNextDirection;

  int newX=snakeBody[0].x;
  int newY=snakeBody[0].y;

  switch (snakeDirection) {
    case SNAKE_UP:
      newY--;
      break;

    case SNAKE_DOWN:
      newY++;
      break;

    case SNAKE_LEFT:
      newX--;
      break;

    case SNAKE_RIGHT:
      newX++;
      break;
  }

  // wall collision
  if (newX<0 || newX>=8 || newY<0 || newY>=32) {
    snakeGameOver=true;
    clearMatrix();
    return;
  }

  // food
  bool ateFood=(newX==snakeFood.x && newY==snakeFood.y);

  // self collision
  int bodyCheckLength=snakeLength;
  
  if (!ateFood) { // tail moves away only if snake does not eat => don't check last segment
    bodyCheckLength--;
  }

  for (int i=0; i<bodyCheckLength; i++) {
    if (snakeBody[i].x==newX && snakeBody[i].y==newY) {
      snakeGameOver=true;
      clearMatrix();
      return;
    }
  }

  // grow
  if (ateFood) {
    if (snakeLength<SNAKE_MAX_LENGTH) {
      snakeLength++;
    }

    score++;

    // increase speed
    if (snakeMoveInterval>70) {
      snakeMoveInterval-=10;
    }

    if (snakeLength<SNAKE_MAX_LENGTH) {
      spawnSnakeFood();
    }

    else {
      snakeGameOver=true;
      clearMatrix();
      beep();
      return;
    }
  }
  
  // move snake
  for (int i=snakeLength-1; i>0; i--) {
    snakeBody[i]=snakeBody[i-1];
  }

  snakeBody[0].x=newX;
  snakeBody[0].y=newY;

  drawSnake();
}

void updateSnakeGameOver() {
  if (!snakeGameOver) {
    return;
  }

  if (millis()-snakeLastFlash>=250) {
    snakeLastFlash=millis();
    snakeFlashState=!snakeFlashState;

    if (snakeFlashState) {
      for (int y=0; y<32; y++) {
        for (int x=0; x<8; x++) {
          setPixel(x, y, true);
        }
      }
    } else {
      clearMatrix();
    }
  }
}

void snake() {
  if (!snakeStarted) {
    startSnake();
  }

  if (snakeGameOver) {
    updateSnakeGameOver();
    return;
  }

  if (paused) {
    return;
  }

  updateSnakeDirection(); // read joystick

  // move at the appropriate speed
  if (millis()-snakeLastMove>=snakeMoveInterval) {
    snakeLastMove=millis();
    moveSnake();
  }
}

// Plane --------------------------------------

void startPlane() {
  planeX=PLANE_START_X;
  planeY=PLANE_START_Y;
  score=0;

  planeGameOver=false;
  planeFlashState=false;
  planeStarted=true;

  planeLastMove=millis();
  enemyLastMove=millis();
  bulletLastMove=millis();
  planeLastFlash=millis();

  for (int i=0; i<MAX_ENEMIES; i++) {
    enemies[i].active=false;
  }

  for (int i=0; i<MAX_BULLETS; i++) {
    bullets[i].active=false;
  }

  clearMatrix();

  spawnPlaneEnemy();
}


void spawnPlaneEnemy() {
  for (int i=0; i<MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].x=random(0, 8);
      enemies[i].y=0;
      enemies[i].active=true;
      return;
    }
  }
}


void drawPlanePlayer() {
  // Plane:
  //   X
  //  XXX
  // XXXXX

  setPixel(planeX, planeY-2, true);

  if (planeX-1>=0) {
    setPixel(planeX-1, planeY-1, true);
  }

  setPixel(planeX, planeY-1, true);

  if (planeX+1<8) {
    setPixel(planeX+1, planeY-1, true);
  }

  if (planeX-2>=0) {
    setPixel(planeX-2, planeY, true);
  }

  if (planeX-1>=0) {
    setPixel(planeX-1, planeY, true);
  }

  setPixel(planeX, planeY, true);

  if (planeX+1<8) {
    setPixel(planeX+1, planeY, true);
  }

  if (planeX+2<8) {
    setPixel(planeX+2, planeY, true);
  }
}


void drawPlaneEnemies() {
  for (int i=0; i<MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      continue;
    }

    int x=enemies[i].x;
    int y=enemies[i].y;

    // Enemy:
    //  X
    // XXX

    setPixel(x, y, true);

    if (x-1>=0) {
      setPixel(x-1, y+1, true);
    }

    setPixel(x, y+1, true);

    if (x+1<8) {
      setPixel(x+1, y+1, true);
    }
  }
}


void drawPlaneBullets() {
  for (int i=0; i<MAX_BULLETS; i++) {
    if (bullets[i].active) {
      setPixel(bullets[i].x, bullets[i].y, true);
    }
  }
}


void drawPlaneScreen() {
  clearMatrix();

  drawPlaneEnemies();
  drawPlaneBullets();
  drawPlanePlayer();
}

void updatePlaneMovement() {
  Joystick js=readJoystick();

  // left/right=vertical movement
  if (joystickLeft(js)) {
    if (planeY>PLANE_MIN_Y) {
      planeY--;
    }
  }

  else if (joystickRight(js)) {
    if (planeY<PLANE_MAX_Y) {
      planeY++;
    }
  }

  // up/down=horizontal movement
  if (joystickUp(js)) {
    if (planeX>PLANE_MIN_X) {
      planeX--;
    }
  }

  else if (joystickDown(js)) {
    if (planeX<PLANE_MAX_X) {
      planeX++;
    }
  }
}

void shoot() {
  if (planeGameOver) {
    startPlane();
    return;
  }

  if (paused) {
    return;
  }

  for (int i=0; i<MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x=planeX;
      bullets[i].y=planeY-3;
      bullets[i].active=true;

      tone(buzzer, 1500, 50);

      return;
    }
  }
}


// move bullets upward
void updatePlaneBullets() {
  for (int i=0; i<MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      continue;
    }

    bullets[i].y--;

    // bullet out of bounds
    if (bullets[i].y<0) {
      bullets[i].active=false;
      continue;
    }

    // check enemy collisions
    for (int j=0; j<MAX_ENEMIES; j++) {
      if (!enemies[j].active) {
        continue;
      }

      int enemyX=enemies[j].x;
      int enemyY=enemies[j].y;

      // enemy occupies:
      //   X
      //  XXX
      bool hit=false;

      if (bullets[i].y==enemyY && bullets[i].x==enemyX) {
        hit=true;
      }

      if (bullets[i].y==enemyY+1 && bullets[i].x>=enemyX-1 && bullets[i].x<=enemyX+1) {
        hit=true;
      }

      if (hit) {
        bullets[i].active=false;
        enemies[j].active=false;

        if (score<999) {
          score++;
        }

        tone(buzzer, 2200, 70);
        break;
      }
    }
  } 
}


// move enemies downward
void updatePlaneEnemies() {
  for (int i=0; i<MAX_ENEMIES; i++) {

    if (!enemies[i].active) {
      continue;
    }

    enemies[i].y++;

    // enemy is close to player
    if (enemies[i].y>=planeY-2) {
      int enemyX=enemies[i].x;
      int enemyY=enemies[i].y;

      // plane occupies planeX-2 to planeX+2
      bool hitX=(enemyX>=planeX-2) && (enemyX<=planeX+2);
      bool hitY=(enemyY>=planeY-2) && (enemyY<=planeY);

      if (hitX && hitY) {
        planeGameOver=true;
        clearMatrix();
        tone(buzzer, 200, 300);
        return;
      }
    }

    // enemy passed player
    if (enemies[i].y>=31) {
      planeGameOver=true;
      clearMatrix();
      tone(buzzer, 200, 300);
      return;
    }
  }
}

// add more enemies
void spawnMorePlaneEnemies() {
  int activeEnemies=0;
  for (int i=0; i<MAX_ENEMIES; i++) {

    if (enemies[i].active) {
      activeEnemies++;
    }
  }

  if (activeEnemies<2) {
    spawnPlaneEnemy();
  }

  if (activeEnemies<MAX_ENEMIES) {
    if (random(0, 100)<20) {
      spawnPlaneEnemy();
    }
  }
}

void updatePlaneGameOver() {
  if (!planeGameOver) {
    return;
  }

  if (millis()-planeLastFlash>=250) {
    planeLastFlash=millis();
    planeFlashState=!planeFlashState;

    if (planeFlashState) {
      for (int y=0; y<32; y++) {
        for (int x=0; x<8; x++) {
          setPixel(x, y, true);
        }
      }
    }

    else {
      clearMatrix();
    }
  }
}


void plane() {
  if (!planeStarted) {
    startPlane();
  }

  if (planeGameOver) {
    updatePlaneGameOver();
    return;
  }

  if (paused) {
    return;
  }

  // move player
  if (millis()-planeLastMove>=planeMoveInterval) {
    planeLastMove=millis();
    updatePlaneMovement();
  }

  // move bullets
  if (millis()-bulletLastMove>=bulletMoveInterval) {
    bulletLastMove=millis();
    updatePlaneBullets();
  }

  // move enemies
  if (millis()-enemyLastMove>=enemyMoveInterval) {
    enemyLastMove=millis();
    updatePlaneEnemies();

    if (!planeGameOver) {
      spawnMorePlaneEnemies();
    }
  }

  drawPlaneScreen();
}


void setup() {
  Serial.begin(9600);

  pinMode(DIG1, OUTPUT);
  pinMode(DIG2, OUTPUT);
  pinMode(DIG3, OUTPUT);

  for (int i=0; i<7; i++) {
    pinMode(segments[i], OUTPUT);
  }

  allDigitsOff();

  pinMode(actionButton, INPUT_PULLUP);

  pinMode(modeButton, INPUT_PULLUP);

  pinMode(joystick_X, INPUT);
  pinMode(joystick_Y, INPUT);
  
  pinMode(buzzer, OUTPUT);

  pinMode(LED_CS, OUTPUT);
  pinMode(LED_CLK, OUTPUT);
  pinMode(LED_DIN, OUTPUT);

  setupMatrix();
  randomSeed(micros());
}

void loop() {
  if (score>999) score=999;
  if (score<0) score=0;

  refreshDisplay();
  readModeButton();

  if (readActionButton()) {
    switch (currentMode) {
      case TETRIS:
        if (tetrisGameOver) {
          startTetris();
        }

        else {
          rotate();
        }
        break;

      case SNAKE:
        if (snakeGameOver) {
          startSnake();
        }
        break;

      case PLANE:
        shoot();
        break;
    }
  }

  switch (currentMode) {
    case TETRIS:
      tetris();
      break;

    case SNAKE:
      snake();
      break;
    
    case PLANE:
      plane();
      break;
  }
}
