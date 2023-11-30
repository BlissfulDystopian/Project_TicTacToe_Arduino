#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>


#pragma region PressureTouch
// define tekanan
#define MINPRESSURE 200
#define MAXPRESSURE 1000

#pragma endregion PressureTouch

#pragma region ColorCodedHex
// define heksadesimal warna
#define BLACK   0x0000
#define WHITE   0xFFFF
#define BLUE    0x001F
#define SMTH    0x0005
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0

#pragma endregion ColorCodedHex

#pragma region Initialization

// Calibrated for Adafruit TFT Shield screen 2.4 inch
int pixel_x, pixel_y;

const int XP = 6, XM = A2, YP = A1, YM = 7;  // Touchscreen pins
const int TS_LEFT = 944, TS_RT = 167, TS_TOP = 908, TS_BOT = 159;

char grid[3][3] = {{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}; // Playspace ketika kosong
char playerSymbols[2] = {' ', ' '};  // menyimpan simbol yang dipilih pemain
int currentPlayer = 0;  // 0 sebagai player1, 1 sebagai player 2

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);  // Kalibrasi ts

void setup() {
  Serial.begin(9600);

  uint16_t ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  Serial.println("Calibrate for your Touch Panel");
  
  if (ID == 0xD3D3) ID = 0x9486;
  
  tft.begin(ID);
  tft.setRotation(1); // Landscape
  tft.fillScreen(BLACK);

  // Player 1 chooses a symbol
  clearScreen();
  playerSymbols[0] = chooseSymbolMenu("P1");
  clearScreen();
  displayNotification("P1 choose: ", playerSymbols[0], CYAN);
  clearScreen();
  delay(200);

  // Player 2 chooses a symbol
  currentPlayer = 1;  // Ganti ke pemain dua
  
  //  Mengantisipasi player 2 memilih symbol yang sama dengan
  //  pemain 1
  while(1) {
    playerSymbols[1] = chooseSymbolMenu("P2");
    if(playerSymbols[1] != playerSymbols[0]) break; 
    clearScreen();
    displayNotification("Symbol's taken!", playerSymbols[1], CYAN);
    clearScreen();
    
  }
  clearScreen();
  displayNotification("P2 choose: ", playerSymbols[1]);
  clearScreen();
  delay(200);

  // Start the game
  currentPlayer = 0;  // Ganti ke pemain satu
  drawGrid();
}

#pragma endregion Initialization

#pragma region GameLoop

void game() {
  bool down = Touch_getXY(); // Ketika koordinat tertentu ditekan

  if (down) {
    int row = pixel_y / 80;
    int col = pixel_x / 80;
    playMove(row, col); // Pemain bergerak
    delay(500);  // Add a delay to avoid registering multiple touches
  }
}

void loop() {
  game();
}

#pragma endregion GameLoop

#pragma region Notification

void displayNotification(const char* message, char symbol) {
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setRotation(0);
  tft.setCursor(50, 100);
  tft.println(message);
  tft.print(symbol);
  delay(2000);
}

void displayNotification(const char* message, char symbol, const int color) {
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setRotation(0);
  tft.setCursor(50, 100);
  tft.println(message);
  tft.print(symbol);
  delay(2000);
}


#pragma endregion Notification

#pragma region Render

void drawGrid() {
  tft.setRotation(1);
  tft.drawRect(0, 0, 240, 320, YELLOW);

  // Gambar grid TTT 3x3
  for (int i = 1; i < 3; i++) {
    tft.drawFastVLine(i * 80, 0, 320, YELLOW);
    tft.drawFastHLine(0, i * 80, 240, YELLOW);
  }
  // Bila grid tidak kosong, print isi dari grid (O / X)
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (grid[i][j] != ' ') {
        tft.setTextSize(3);
        tft.setCursor(j * 80 + 20, i * 80 + 20);
        tft.setTextColor(CYAN);
        tft.print(grid[i][j]);
      }
    }
  }
}

void clearScreen() {
  tft.fillScreen(SMTH); // Menghapus background
}

#pragma endregion Render

#pragma region Movement

void playMove(int row, int col) {
  // Jika grid yang dipilih masih kosong, isi dengan tanda current player
  if (grid[row][col] == ' ') {
    grid[row][col] = playerSymbols[currentPlayer];
    drawGrid();
    checkWinCondition();
    switchPlayer();
  }
}

#pragma endregion Movement

#pragma region SwitchPlayer

void switchPlayer() {
  // Memindahkan currentPlayer ke player selanjutnya
  currentPlayer = (currentPlayer + 1) % 2;
}

#pragma endregion SwitchPlayer

#pragma region TouchRegister

// Fungsi boolean untuk mendeteksi adanya sentuhan di layar
bool Touch_getXY() {
  TSPoint p = ts.getPoint(); 
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.y, TS_LEFT, TS_RT, 0, tft.width()); // Register adanya tekanan di titik x
    pixel_y = map(p.x, TS_TOP, TS_BOT, 0, tft.height()); // Register adanya tekanan di titik y
  }
  return pressed;
}

#pragma endregion TouchRegister

#pragma region CheckWinning

void checkWinCondition() {
  // Cek Baris
  for (int i = 0; i < 3; i++) {
    if (grid[i][0] == grid[i][1] && grid[i][1] == grid[i][2] && grid[i][0] != ' ') {
      announceWinner(grid[i][0]);
      return;
    }
  }

  // Cek kolom
  for (int j = 0; j < 3; j++) {
    if (grid[0][j] == grid[1][j] && grid[1][j] == grid[2][j] && grid[0][j] != ' ') {
      announceWinner(grid[0][j]);
      return;
    }
  }

  // Cek diagonals
  if ((grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2] && grid[0][0] != ' ') ||
      (grid[0][2] == grid[1][1] && grid[1][1] == grid[2][0] && grid[0][2] != ' ')) {
    announceWinner(grid[1][1]);
    return;
  }

  // Check bila draw
  bool draw = true;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (grid[i][j] == ' ') {
        draw = false;
        break;
      }
    }
    if (!draw) {
      break;
    }
  }

  if (draw) {
    announceDraw();
  }
}

#pragma endregion CheckWinning

#pragma region AnnouncementSystem

void announceWinner(char symbol) {
  tft.setTextSize(2);
  

  if (symbol == playerSymbols[0]) {
    tft.setTextColor(BLUE);
    tft.setRotation(0);
    tft.setCursor(50, 250);
    tft.print("P1(");
    tft.print(symbol);
    tft.print(") wins!");
  } else if (symbol == playerSymbols[1]) {
    tft.setTextColor(GREEN);
    tft.setRotation(0);
    tft.setCursor(50, 260);
    tft.print("P2(");
    tft.print(symbol);
    tft.print(") wins!");
  }

  delay(2000);
  resetGame();
}
void announceDraw() {
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setRotation(0);
  tft.setCursor(50, 250);
  tft.print("It's a draw!");
  delay(2000);
  resetGame();
}

#pragma endregion AnnouncementSystem

#pragma region Reset

void resetGame() {
  // Mereset isi grid
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      grid[i][j] = ' ';
    }
  }
  clearScreen(); // menghapus gambar di layar
  
  setup(); // Kembali ke fungsi setup
}
#pragma endregion Reset

#pragma region ChooseSymbol

char chooseSymbolMenu(const char* playerName) {
  char chosenSymbol = ' ';    // Inisialisasi awal (kosong)
  bool symbolChosen = false; 

  // Ketika belum memilih simbol, maka akan diulangi 
  // sampai simbol dipilih oleh pemain
  while (!symbolChosen) {
    tft.setTextSize(2);
    tft.setTextColor(CYAN);
    tft.setRotation(0);
    tft.setCursor(50, 20);
    tft.print(playerName);
    tft.print(" choose: ");
    tft.setCursor(50, 100);
    tft.print("1. X");
    tft.setCursor(50, 150);
    tft.print("2. O");
    tft.setCursor(50, 200);
    tft.print("3. Z");
    tft.setCursor(50, 250);
    tft.print("4. T");

    // Menunggu Input
    while (!Touch_getXY()) {
      delay(10);  // Delay digunakan agar tidak meregister sentuhan ganda
    }

    // Determine the selected option based on touch coordinates

    // Menentukan pilihan pemain dalam bentuk posisi sentuhan
    // Pada koordinat y sama dan x yang berbeda beda
    if (pixel_y > 20 && pixel_y < 200) {
      if (pixel_x > 50 && pixel_x < 90) {
        chosenSymbol = 'X';
        symbolChosen = true;
      } else if (pixel_x > 100 && pixel_x < 140) {
        chosenSymbol = 'O';
        symbolChosen = true;
      } else if (pixel_x > 150 && pixel_x < 170) {
        chosenSymbol = 'Z';
        symbolChosen = true;
      } else if (pixel_x > 180 && pixel_x < 220) {
        chosenSymbol = 'T';
        symbolChosen = true;
      }
    }

    // Delay digunakan untuk menghindari program dari
    // meregister sentuhan ganda
    delay(200);
  }
  return chosenSymbol;
}

#pragma endregion ChooseSymbol
