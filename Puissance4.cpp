#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_PIN_1 7
#define LED_PIN_2 8
#define LED_PIN_3 9
#define LED_PIN_4 10
#define LED_PIN_5 11
#define LED_PIN_6 12
#define LED_PIN_7 13
#define NUM_LEDS 7
#define LED_BRIGHTNESS 50

#define BUTTON_1_PIN 0
#define BUTTON_2_PIN 1
#define BUTTON_3_PIN 2
#define BUTTON_4_PIN 3
#define BUTTON_5_PIN 4
#define BUTTON_6_PIN 5
#define BUTTON_7_PIN 6

Adafruit_NeoPixel pixels[NUM_LEDS];
#define DELAYVAL 500


enum class CellState {
  EMPTY,
  RED,
  YELLOW
};

enum class Difficulty {
  EASY,
  MEDIUM,
  HARD
};

CellState board[7][6];
CellState currentPlayer;
Difficulty currentDifficulty;
int scorePlayer1 = 0;
int scorePlayer2 = 0;

void setup();
void setupGame();
void fillAllLeds();
void playerMove();
void botMove(CellState player);
int findWinningMove(CellState player);
bool isValidMove(int column);
int minimax(int column, int depth, bool isMaximizingPlayer, CellState player);
bool checkWin(CellState player);
bool checkDraw();
void loop();

void setup() {
  Serial.begin(9600); // Initialize Serial communication with 9600 baud rate

  randomSeed(analogRead(0));

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  pinMode(BUTTON_5_PIN, INPUT_PULLUP);
  pinMode(BUTTON_6_PIN, INPUT_PULLUP);
  pinMode(BUTTON_7_PIN, INPUT_PULLUP);

  pixels[0] = Adafruit_NeoPixel(6, LED_PIN_1, NEO_GRB + NEO_KHZ800);
  pixels[1] = Adafruit_NeoPixel(6, LED_PIN_2, NEO_GRB + NEO_KHZ800);
  pixels[2] = Adafruit_NeoPixel(6, LED_PIN_3, NEO_GRB + NEO_KHZ800);
  pixels[3] = Adafruit_NeoPixel(6, LED_PIN_4, NEO_GRB + NEO_KHZ800);
  pixels[4] = Adafruit_NeoPixel(6, LED_PIN_5, NEO_GRB + NEO_KHZ800);
  pixels[5] = Adafruit_NeoPixel(6, LED_PIN_6, NEO_GRB + NEO_KHZ800);
  pixels[6] = Adafruit_NeoPixel(6, LED_PIN_7, NEO_GRB + NEO_KHZ800);
  
  setupGame();
}

void setupGame() {
  for (int x = 0; x < 7; x++) {
    for (int y = 0; y < 6; y++) {
      board[x][y] = CellState::EMPTY;
    }
  }

  currentPlayer = (random(2) == 0) ? CellState::RED : CellState::YELLOW;
}

void fillAllLeds() {
  for (int x = 0; x < 7; x++) {
    for (int y = 0; y < 6; y++) {
      if (board[x][y] == CellState::RED) {
        pixels[x].setPixelColor(y, pixels[0].Color(255, 0, 0));
      } else if (board[x][y] == CellState::YELLOW) {
        pixels[x].setPixelColor(y, pixels[0].Color(255, 255, 0));
      } else {
        pixels[x].setPixelColor(y, pixels[0].Color(0, 0, 255));
      }
    }
    pixels[x].show();
  }
}

void playerMove() {
  int column = -1;
  bool validMove = false;

  while (!validMove) {
    
    if (digitalRead(BUTTON_1_PIN) == 0) {
      column = 1;
    } else if (digitalRead(BUTTON_2_PIN) == 0) {
      column = 2;
    } else if (digitalRead(BUTTON_3_PIN) == 0) {
      column = 3;
    } else if (digitalRead(BUTTON_4_PIN) == 0) {
      column = 4;
    } else if (digitalRead(BUTTON_5_PIN) == 0) {
      column = 5;
    } else if (digitalRead(BUTTON_6_PIN) == 0) {
      column = 6;
    } else if (digitalRead(BUTTON_7_PIN) == 0) {
      column = 7;
    }

    if (column != -1 && column >= 1 && column <= 7) {
      for (int y = 5; y >= 0; y--) {
        if (board[column - 1][y] == CellState::EMPTY) {
          board[column - 1][y] = currentPlayer;
          validMove = true;
          break;
        }
      }
    }

    delay(50);
  }
}

void botMove(CellState player) {
  int column = -1;
  bool validMove = false;

  int currentPlayerScore = (player == CellState::RED) ? scorePlayer1 : scorePlayer2;

  if (currentPlayerScore < 2) {
    currentDifficulty = Difficulty::EASY;
  } else if (currentPlayerScore < 4) {
    currentDifficulty = Difficulty::MEDIUM;
  } else {
    currentDifficulty = Difficulty::HARD;
  }

  while (!validMove) {
    switch (currentDifficulty) {
      case Difficulty::EASY:
        column = random(1, 8);
        break;
      case Difficulty::MEDIUM:
        column = findWinningMove(player);
        if (column == -1) {
          column = random(1, 8);
        }
        break;
      case Difficulty::HARD:
        int depth = 4;
        int bestScore = INT8_MIN; // Replace INT_MIN
        for (int col = 1; col <= 7; col++) {
          if (isValidMove(col)) {
            int score = minimax(col, depth, false, player);
            if (score > bestScore) {
              bestScore = score;
              column = col;
            }
          }
        }
        break;
    }

    if (column != -1 && column >= 1 && column <= 7) {
      for (int y = 5; y >= 0; y--) {
        if (board[column - 1][y] == CellState::EMPTY) {
          board[column - 1][y] = currentPlayer;
          validMove = true;
          break;
        }
      }
    }
  }
}

int findWinningMove(CellState player) {
  for (int col = 1; col <= 7; col++) {
    if (isValidMove(col)) {
      for (int y = 5; y >= 0; y--) {
        if (board[col - 1][y] == CellState::EMPTY) {
          board[col - 1][y] = player;
          if (checkWin(player)) {
            board[col - 1][y] = CellState::EMPTY;
            return col;
          }
          board[col - 1][y] = CellState::EMPTY;
          break;
        }
      }
    }
  }
  return -1;
}

bool isValidMove(int column) {
  if (column < 1 || column > 7) {
    return false;
  }
  return board[column - 1][0] == CellState::EMPTY;
}

int minimax(int column, int depth, bool isMaximizingPlayer, CellState player) {
  if (depth == 0 || checkWin(CellState::RED) || checkWin(CellState::YELLOW) || checkDraw()) {
    if (checkWin(CellState::YELLOW)) {
      return (player == CellState::YELLOW) ? 100 : -100;
    } else if (checkWin(CellState::RED)) {
      return (player == CellState::RED) ? 100 : -100;
    } else {
      return 0;
    }
  }

  if (isMaximizingPlayer) {
    int bestScore = INT8_MIN; // Replace INT_MIN
    for (int col = 1; col <= 7; col++) {
      if (isValidMove(col)) {
        for (int y = 5; y >= 0; y--) {
          if (board[col - 1][y] == CellState::EMPTY) {
            board[col - 1][y] = CellState::YELLOW;
            int score = minimax(col, depth - 1, false, player);
            board[col - 1][y] = CellState::EMPTY;
            bestScore = max(bestScore, score);
            break;
          }
        }
      }
    }
    return bestScore;
  } else {
    int bestScore = INT8_MAX; // Replace INT_MAX
    for (int col = 1; col <= 7; col++) {
      if (isValidMove(col)) {
        for (int y = 5; y >= 0; y--) {
          if (board[col - 1][y] == CellState::EMPTY) {
            board[col - 1][y] = CellState::RED;
            int score = minimax(col, depth - 1, true, player);
            board[col - 1][y] = CellState::EMPTY;
            bestScore = min(bestScore, score);
            break;
          }
        }
      }
    }
    return bestScore;
  }
}

bool checkWin(CellState player) {
  // Check rows
  for (int y = 0; y < 6; y++) {
    for (int x = 0; x < 4; x++) {
      if (board[x][y] == player && board[x + 1][y] == player && board[x + 2][y] == player && board[x + 3][y] == player) {
        return true;
      }
    }
  }

  // Check columns
  for (int x = 0; x < 7; x++) {
    for (int y = 0; y < 3; y++) {
      if (board[x][y] == player && board[x][y + 1] == player && board[x][y + 2] == player && board[x][y + 3] == player) {
        return true;
      }
    }
  }

  // Check diagonals up
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 3; y++) {
      if (board[x][y] == player && board[x + 1][y + 1] == player && board[x + 2][y + 2] == player && board[x + 3][y + 3] == player) {
        return true;
      }
    }
  }

  // Check diagonals down
  for (int x = 0; x < 4; x++) {
    for (int y = 3; y < 6; y++) {
      if (board[x][y] == player && board[x + 1][y - 1] == player && board[x + 2][y - 2] == player && board[x + 3][y - 3] == player) {
        return true;
      }
    }
  }

  return false;
}

bool checkDraw() {
  for (int x = 0; x < 7; x++) {
    for (int y = 0; y < 6; y++) {
      if (board[x][y] == CellState::EMPTY) {
        return false;
      }
    }
  }

  if (!checkWin(CellState::RED) && !checkWin(CellState::YELLOW)) {
    return true;
  }

  return false;
}

void loop() {
  fillAllLeds();
  if (currentPlayer == CellState::RED) {
    playerMove();
    currentPlayer = CellState::YELLOW;
  } else {
    botMove(CellState::YELLOW);
    currentPlayer = CellState::RED;
  }
  if (checkWin(CellState::RED)) {
    scorePlayer1++;
    Serial.println("Player 1 wins!");
    setup();
    setupGame();
  } else if (checkWin(CellState::YELLOW)) {
    scorePlayer2++;
    Serial.println("Bot wins!");
    setup();
    setupGame();
  } else if (checkDraw()) {
    Serial.println("It's a draw!");
    setup();
    setupGame();
  }
  fillAllLeds();
  delay(DELAYVAL);
}
