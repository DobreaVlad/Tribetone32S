#include <Adafruit_NeoPixel.h>
#include <Control_Surface.h>
#include <MIDI_Interfaces/BluetoothMIDI_Interface.hpp>
#include <Adafruit_MPR121.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MUX74HC4067.h>

//_________________________Task on core 2
TaskHandle_t Task1;

//_______________________________OLED
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
unsigned long lastoled = 0;
unsigned long oledtime = 100;
bool refreshdisplay = true;

//_____________________________Encoders
MUX74HC4067 mux(18, 15, 4, 16, 17);
int encoders[13][25] = { 0 };
int aState1;
int aLastState1;
int aState2;
int aLastState2;
int aState3;
int aLastState3;
int aState4;
int aLastState4;
int aState5;
int aLastState5;
int aState6;
int aLastState6;
int aState7;
int aLastState7;
int aState8;
int aLastState8;

int page = 0;

//_____________Menu Encoder
int CLK = 13;
int DT = 27;
int aState;
int aLastState;

//_____________________________Touch
#define _BV(bit) (1 << (bit))
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
uint16_t lasttouched1 = 0;
uint16_t currtouched1 = 0;
uint16_t lasttouched2 = 0;
uint16_t currtouched2 = 0;

//____________________________Mute
bool mute[13] = { false };

//______________________________________________________Chain
bool chainstate[13] = { false };
int chaintrack[13] = { 4 };




//_________________________________Tempo Pot
int tempo = 130;
unsigned long us_per_tick = (unsigned long)(1e6 / (tempo * 24 / 60));


//________________________________________________Sequencer
#define CHANNELS 14
#define TRACKS 4
#define STEPS 17

bool global = false;
byte current_channel = 1;
unsigned long last_tick = 0;
bool onoffstate = false;
bool stepstate = false;
bool mutestate = false;
bool button1 = false;
bool button2 = false;
bool button3 = false;
bool button4 = false;
const int MAX_STEP = 16;
const int NUM_LEDS = 16;
const int LED_OFFSET = 16;
const int TICK_THRESHOLD = 6;
int tick_count = 0;
int current_step = 1;
int current_led = 16;
int track[CHANNELS] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
int steptrack[CHANNELS] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
bool sequencer[CHANNELS][TRACKS][STEPS] = { 0 };
const char* text[13] = { " ", "1 KICK", "2 SNARE", "3 CLAP", "4 CL-HAT", "5 OP-HAT", "6 PERC1", "7 PERC2", "8 PERC3", "9 PERC4", "10 ATMOS", "11 MELODY2", "12 MELODY3" };

//_________________________________________________________Notes
byte Note[14] = { 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36 };
Channel myChannel[14] = { CHANNEL_1, CHANNEL_1, CHANNEL_2, CHANNEL_3, CHANNEL_4, CHANNEL_5, CHANNEL_6, CHANNEL_7, CHANNEL_8, CHANNEL_9, CHANNEL_10, CHANNEL_11, CHANNEL_12, CHANNEL_13 };
MIDIAddress Notes[14] = {
  { Note[0], myChannel[0] },
  { Note[1], myChannel[1] },
  { Note[2], myChannel[2] },
  { Note[3], myChannel[3] },
  { Note[4], myChannel[4] },
  { Note[5], myChannel[5] },
  { Note[6], myChannel[6] },
  { Note[7], myChannel[7] },
  { Note[8], myChannel[8] },
  { Note[9], myChannel[9] },
  { Note[10], myChannel[10] },
  { Note[11], myChannel[11] },
  { Note[12], myChannel[12] },
  { Note[13], myChannel[13] }
};


//___________________________ BLE
BluetoothMIDI_Interface midi;

//___________________________ Neopixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(44, 23, NEO_GRB + NEO_KHZ800);

//____________________________________Step activated
int velocities[13] = { 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127 };
int colors[13][3] = { { 0, 0, 0 }, { 40, 0, 250 }, { 230, 20, 255 }, { 150, 100, 0 }, { 0, 255, 0 }, { 50, 255, 100 }, { 250, 75, 0 }, { 250, 75, 0 }, { 250, 75, 0 }, { 250, 75, 0 }, { 250, 75, 255 }, { 250, 100, 255 }, { 250, 125, 255 } };

//Bitmap
const unsigned char bitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x03, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf0, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x04, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe7, 0x00, 0x18, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xef, 0xe0, 0x31, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7, 0xf0, 0x73, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xf7, 0xf0, 0x77, 0xcf, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x77, 0xf9, 0xe7, 0xce, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x7f, 0xeb, 0xe7, 0xde, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x3d, 0xdf, 0xcb, 0x1c, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x3c, 0x3e, 0x3f, 0xc0, 0x38, 0x7c, 0x38, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xdf, 0x1f, 0x7e, 0x00, 0x38, 0xfd, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0xfe, 0x80, 0x37, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0xfe, 0x00, 0x6f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfc, 0x00, 0x5f, 0xfe, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfc, 0x00, 0xdf, 0xf8, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xfc, 0x00, 0xcf, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfc, 0x01, 0x8b, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x7e, 0xf8, 0x03, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x40, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x40, 0x1e, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x03, 0xc0, 0x0f, 0xc0, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x07, 0xe0, 0x3f, 0x60, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x03, 0xff, 0xff, 0xc0, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x01, 0xff, 0xff, 0x80, 0x39, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x03, 0xff, 0xfe, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x03, 0x80, 0x00, 0x40, 0x10, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x03, 0x40, 0x02, 0xc0, 0x1c, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x02, 0x40, 0x02, 0x80, 0x1c, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x06, 0xf0, 0x00, 0x00, 0x02, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x30, 0x07, 0xc0, 0x00, 0xc0, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x70, 0x03, 0x80, 0x00, 0x80, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x03, 0x00, 0x02, 0x80, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x08, 0x40, 0x03, 0xe0, 0x0b, 0x80, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x08, 0x40, 0x01, 0x60, 0x09, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x10, 0xc0, 0x01, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x10, 0x40, 0x0f, 0x80, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x20, 0x01, 0x8f, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 0x01, 0xc6, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void setup() {


  //______________________________Neopixels
  strip.setBrightness(35);

  strip.begin();
  for (int i = 0; i < 44; i++) strip.setPixelColor(i, strip.Color(250, 250, 250));
  strip.show();
  //_____________________________OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("OLED failed to initialize"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, 128, 64, WHITE);
  display.display();

  Serial.begin(115200);

  //______________________________________________________________Encoders
  mux.signalPin(5, INPUT, DIGITAL);
  aLastState1 = mux.read(0);
  aLastState2 = mux.read(2);
  aLastState3 = mux.read(4);
  aLastState4 = mux.read(6);
  aLastState5 = mux.read(8);
  aLastState6 = mux.read(10);
  aLastState7 = mux.read(12);
  aLastState8 = mux.read(14);

  aLastState = digitalRead(CLK);

  //-------------------------------Volume
  encoders[1][2] = { 63 };
  encoders[2][2] = { 63 };
  encoders[3][2] = { 63 };
  encoders[4][2] = { 63 };
  encoders[5][2] = { 63 };
  encoders[6][2] = { 63 };
  encoders[7][2] = { 63 };
  encoders[8][2] = { 63 };
  encoders[9][2] = { 63 };
  encoders[10][2] = { 63 };
  encoders[11][2] = { 63 };
  encoders[12][2] = { 63 };
  //-------------------------------Pitch
  encoders[1][3] = { 63 };
  encoders[2][3] = { 63 };
  encoders[3][3] = { 63 };
  encoders[4][3] = { 63 };
  encoders[5][3] = { 63 };
  encoders[6][3] = { 63 };
  encoders[7][3] = { 63 };
  encoders[8][3] = { 63 };
  encoders[9][3] = { 63 };
  encoders[10][3] = { 63 };
  encoders[11][3] = { 63 };
  encoders[12][3] = { 63 };
  //-------------------------------High
  encoders[1][4] = { 63 };
  encoders[2][4] = { 63 };
  encoders[3][4] = { 63 };
  encoders[4][4] = { 63 };
  encoders[5][4] = { 63 };
  encoders[6][4] = { 63 };
  encoders[7][4] = { 63 };
  encoders[8][4] = { 63 };
  encoders[9][4] = { 63 };
  encoders[10][4] = { 63 };
  encoders[11][4] = { 63 };
  encoders[12][4] = { 63 };
  //-------------------------------Mid
  encoders[1][5] = { 63 };
  encoders[2][5] = { 63 };
  encoders[3][5] = { 63 };
  encoders[4][5] = { 63 };
  encoders[5][5] = { 63 };
  encoders[6][5] = { 63 };
  encoders[7][5] = { 63 };
  encoders[8][5] = { 63 };
  encoders[9][5] = { 63 };
  encoders[10][5] = { 63 };
  encoders[11][5] = { 63 };
  encoders[12][5] = { 63 };
  //-------------------------------Low
  encoders[1][6] = { 63 };
  encoders[2][6] = { 63 };
  encoders[3][6] = { 63 };
  encoders[4][6] = { 63 };
  encoders[5][6] = { 63 };
  encoders[6][6] = { 63 };
  encoders[7][6] = { 63 };
  encoders[8][6] = { 63 };
  encoders[9][6] = { 63 };
  encoders[10][6] = { 63 };
  encoders[11][6] = { 63 };
  encoders[12][6] = { 63 };
  //-------------------------------Saturator
  encoders[1][15] = { 63 };
  encoders[2][15] = { 63 };
  encoders[3][15] = { 63 };
  encoders[4][15] = { 63 };
  encoders[5][15] = { 63 };
  encoders[6][15] = { 63 };
  encoders[7][15] = { 63 };
  encoders[8][15] = { 63 };
  encoders[9][15] = { 63 };
  encoders[10][15] = { 63 };
  encoders[11][15] = { 63 };
  encoders[12][15] = { 63 };


  //______________________________________________________________Touch 1
  if (!cap1.begin(0x5A)) {
    Serial.println("Touch1 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("Touch1 found!");

  //______________________________________________________________Touch 2
  if (!cap2.begin(0x5c)) {
    Serial.println("Touch2 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("Touch2 found!");

  //Sequencer
  sequencer[1][4][1] = { 1 };
  sequencer[1][4][2] = { 0 };
  sequencer[1][4][3] = { 0 };
  sequencer[1][4][4] = { 0 };
  sequencer[1][4][5] = { 1 };
  sequencer[1][4][6] = { 0 };
  sequencer[1][4][7] = { 0 };
  sequencer[1][4][8] = { 0 };
  sequencer[1][4][9] = { 1 };
  sequencer[1][4][10] = { 0 };
  sequencer[1][4][11] = { 0 };
  sequencer[1][4][12] = { 0 };
  sequencer[1][4][13] = { 1 };
  sequencer[1][4][14] = { 0 };
  sequencer[1][4][15] = { 0 };
  sequencer[1][4][16] = { 0 };

  sequencer[1][3][1] = { 1 };
  sequencer[1][3][2] = { 0 };
  sequencer[1][3][3] = { 1 };
  sequencer[1][3][4] = { 0 };
  sequencer[1][3][5] = { 1 };
  sequencer[1][3][6] = { 0 };
  sequencer[1][3][7] = { 0 };
  sequencer[1][3][8] = { 0 };
  sequencer[1][3][9] = { 1 };
  sequencer[1][3][10] = { 0 };
  sequencer[1][3][11] = { 0 };
  sequencer[1][3][12] = { 0 };
  sequencer[1][3][13] = { 1 };
  sequencer[1][3][14] = { 0 };
  sequencer[1][3][15] = { 0 };
  sequencer[1][3][16] = { 0 };

  sequencer[1][2][1] = { 1 };
  sequencer[1][2][2] = { 0 };
  sequencer[1][2][3] = { 0 };
  sequencer[1][2][4] = { 0 };
  sequencer[1][2][5] = { 1 };
  sequencer[1][2][6] = { 0 };
  sequencer[1][2][7] = { 0 };
  sequencer[1][2][8] = { 0 };
  sequencer[1][2][9] = { 1 };
  sequencer[1][2][10] = { 0 };
  sequencer[1][2][11] = { 0 };
  sequencer[1][2][12] = { 0 };
  sequencer[1][2][13] = { 1 };
  sequencer[1][2][14] = { 0 };
  sequencer[1][2][15] = { 0 };
  sequencer[1][2][16] = { 0 };

  sequencer[1][1][1] = { 1 };
  sequencer[1][1][2] = { 1 };
  sequencer[1][1][3] = { 1 };
  sequencer[1][1][4] = { 0 };
  sequencer[1][1][5] = { 1 };
  sequencer[1][1][6] = { 0 };
  sequencer[1][1][7] = { 0 };
  sequencer[1][1][8] = { 0 };
  sequencer[1][1][9] = { 1 };
  sequencer[1][1][10] = { 0 };
  sequencer[1][1][11] = { 0 };
  sequencer[1][1][12] = { 0 };
  sequencer[1][1][13] = { 1 };
  sequencer[1][1][14] = { 0 };
  sequencer[1][1][15] = { 1 };
  sequencer[1][1][16] = { 0 };

  sequencer[2][4][1] = { 0 };
  sequencer[2][4][2] = { 0 };
  sequencer[2][4][3] = { 0 };
  sequencer[2][4][4] = { 0 };
  sequencer[2][4][5] = { 1 };
  sequencer[2][4][6] = { 0 };
  sequencer[2][4][7] = { 0 };
  sequencer[2][4][8] = { 0 };
  sequencer[2][4][9] = { 0 };
  sequencer[2][4][10] = { 0 };
  sequencer[2][4][11] = { 0 };
  sequencer[2][4][12] = { 0 };
  sequencer[2][4][13] = { 1 };
  sequencer[2][4][14] = { 0 };
  sequencer[2][4][15] = { 0 };
  sequencer[2][4][16] = { 0 };

  sequencer[4][4][1] = { 0 };
  sequencer[4][4][2] = { 0 };
  sequencer[4][4][3] = { 1 };
  sequencer[4][4][4] = { 0 };
  sequencer[4][4][5] = { 0 };
  sequencer[4][4][6] = { 0 };
  sequencer[4][4][7] = { 1 };
  sequencer[4][4][8] = { 0 };
  sequencer[4][4][9] = { 0 };
  sequencer[4][4][10] = { 0 };
  sequencer[4][4][11] = { 1 };
  sequencer[4][4][12] = { 0 };
  sequencer[4][4][13] = { 0 };
  sequencer[4][4][14] = { 0 };
  sequencer[4][4][15] = { 1 };
  sequencer[4][4][16] = { 0 };



  //________________________________Control Surface
  midi.setName("TribeMachine 32S");
  Control_Surface.begin();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  delay(2000);

  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    80000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    1);        /* pin task to core 0 */


  updateDisplay();
}

void Task1code(void* pvParameters) {
  for (;;) {

    if (onoffstate) {
      //_________________________Midi Clock
      if (micros() - last_tick >= us_per_tick) {
        last_tick += us_per_tick;
        midi.sendRealTime(0xF8);
        //____________________________________Sequencer MIDI Note
        if (tick_count == 0) {
          for (int i = 1; i <= 12; i++) {
            if (sequencer[i][track[i]][current_step] == 1) {
              midi.sendNoteOn(Notes[i], velocities[i]);
              midi.sendNoteOff(Notes[i], velocities[i]);
            }
          }
          //____________________________________________________Chain steps
          if (current_step == 16) {
            for (int j = 1; j < 13; j++)
              if (chainstate[j]) {
                track[j] = (track[j] + 2) % 4 + 1;
              }
          }
        }
        tick_count++;
      }
      //__________________________________________________________Neopixel sequencer
      if (tick_count == TICK_THRESHOLD) {
        current_step = (current_step % MAX_STEP) + 1;
        current_led = (current_led - LED_OFFSET + 1) % NUM_LEDS + LED_OFFSET;
        tick_count = 0;
      }
    } else strip.setPixelColor(43, strip.Color(255, 0, 0));
  }
}

void loop() {

  strip.clear();

  //_________________________________________Track LED
  strip.setPixelColor(track[current_channel] + 31, strip.Color(150, 150, 0));

  //_________________________________________________Step Pattern Activated
  if (stepstate) {
    strip.setPixelColor(41, strip.Color(0, 0, 255));
    strip.setPixelColor(steptrack[current_channel] + 31, strip.Color(0, 0, 250));
    for (int i = 16; i < 32; i++)
      if (sequencer[current_channel][steptrack[current_channel]][i - 15] == 1) strip.setPixelColor(i, strip.Color(0, 0, 255));
  } else
    for (int i = 1; i <= 16; i++)
      if (sequencer[current_channel][track[current_channel]][i] == 1) strip.setPixelColor(i + 15, strip.Color(255, 0, 0));

  //_______________________________________________________________Sequencer LEDs
  if (current_led == 16 | current_led == 20 | current_led == 24 | current_led == 28) strip.setPixelColor(43, strip.Color(0, 255, 0));

  //_____________________________________________________Instrument LED always on/Mute
  if (mutestate) {
    stepstate = false;
    strip.setPixelColor(39, strip.Color(255, 0, 0));
    for (int i = 1; i <= 12; i++)
      if (!mute[i]) strip.setPixelColor(16 - i, strip.Color(colors[i][0], colors[i][1], colors[i][2]));
  }

  else
    strip.setPixelColor(16 - current_channel, strip.Color(colors[current_channel][0], colors[current_channel][1], colors[current_channel][2]));


  if (onoffstate) {
    //________________________________________________Stepper LED
    strip.setPixelColor(current_led, strip.Color(255, 255, 255));

    //____________________________________Sequencer MIDI Note
    for (int i = 1; i <= 12; i++)
      if (sequencer[i][track[i]][current_step] == 1)
        strip.setPixelColor(16 - i, strip.Color(colors[i][0], colors[i][1], colors[i][2]));
  }

  if (button4) strip.setPixelColor(0, strip.Color(250, 255, 250));



  //_______________________________________Chain LED
  if (chainstate[current_channel]) strip.setPixelColor(37, strip.Color(250, 75, 0));

  



  strip.show();

  Control_Surface.loop();

  currtouched1 = cap1.touched();
  currtouched2 = cap2.touched();

  //_______________________________________________________Encoder1
  aState1 = mux.read(0);
  if (aState1 != aLastState1) {
    if (mux.read(1) != aState1) {
      if (page == 0) {
        Note[current_channel]++;
        if (Note[current_channel] == 52) Note[current_channel] = 36;
        MIDIAddress newAddress = { Note[current_channel], myChannel[current_channel] };
        Notes[current_channel] = newAddress;
        if (!onoffstate) {
          midi.sendNoteOn(Notes[current_channel], 127);
          midi.sendNoteOff(Notes[current_channel], 127);
        }
        display.fillRect(46, 17, 12, 10, BLACK);
        display.setCursor(46, 17);
        display.setTextColor(WHITE);
        display.print(Note[current_channel] - 35);

      } else {
        if (encoders[current_channel][1 + page] <= 124) encoders[current_channel][1 + page] += 3;
        midi.sendControlChange({ 70 + page, myChannel[current_channel] }, encoders[current_channel][1 + page]);

        display.fillRect(24, 15, 36, 10, BLACK);
        display.fillRect(24, 15, map(encoders[current_channel][1 + page], 0, 127, 0, 37), 10, WHITE);
      }
    }


    else {
      if (page == 0) {
        Note[current_channel]--;
        if (Note[current_channel] == 35) Note[current_channel] = 51;
        MIDIAddress newAddress = { Note[current_channel], myChannel[current_channel] };
        Notes[current_channel] = newAddress;
        if (!onoffstate) {
          midi.sendNoteOn(Notes[current_channel], 127);
          midi.sendNoteOff(Notes[current_channel], 127);
        }
        display.fillRect(46, 17, 12, 10, BLACK);
        display.setCursor(46, 17);
        display.setTextColor(WHITE);
        display.print(Note[current_channel] - 35);
      } else {

        if (encoders[current_channel][1 + page] > 0) encoders[current_channel][1 + page] -= 3;
        midi.sendControlChange({ 70 + page, myChannel[current_channel] }, encoders[current_channel][1 + page]);
        display.fillRect(24, 15, 36, 10, BLACK);
        display.fillRect(24, 15, map(encoders[current_channel][1 + page], 0, 127, 0, 37), 10, WHITE);
      }
    }

    aLastState1 = aState1;
    refreshdisplay = 1;
  }

  //_______________________________________________________Encoder2
  aState2 = mux.read(3);

  if (aState2 != aLastState2) {
    if (mux.read(2) != aState2) {
      if (encoders[current_channel][2 + page] <= 124) encoders[current_channel][2 + page] += 3;
      midi.sendControlChange({ 71 + page, myChannel[current_channel] }, encoders[current_channel][2 + page]);


    } else {
      if (encoders[current_channel][2 + page] > 0) encoders[current_channel][2 + page] -= 3;
      midi.sendControlChange({ 71 + page, myChannel[current_channel] }, encoders[current_channel][2 + page]);
    }
    aLastState2 = aState2;
    display.fillRect(24, 28, 36, 10, BLACK);
    display.fillRect(24, 28, map(encoders[current_channel][2 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }

  //_______________________________________________________Encoder3
  aState3 = mux.read(4);

  if (aState3 != aLastState3) {
    if (mux.read(5) != aState3) {
      if (encoders[current_channel][3 + page] <= 124) encoders[current_channel][3 + page] += 3;
      midi.sendControlChange({ 72 + page, myChannel[current_channel] }, encoders[current_channel][3 + page]);



    } else {
      if (encoders[current_channel][3 + page] > 0) encoders[current_channel][3 + page] -= 3;
      midi.sendControlChange({ 72 + page, myChannel[current_channel] }, encoders[current_channel][3 + page]);
    }
    aLastState3 = aState3;
    display.fillRect(24, 41, 36, 10, BLACK);
    display.fillRect(24, 41, map(encoders[current_channel][3 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }
  //_______________________________________________________Encoder4
  aState4 = mux.read(6);

  if (aState4 != aLastState4) {
    if (mux.read(7) != aState4) {
      if (encoders[current_channel][4 + page] <= 124) encoders[current_channel][4 + page] += 3;
      midi.sendControlChange({ 73 + page, myChannel[current_channel] }, encoders[current_channel][4 + page]);



    } else {
      if (encoders[current_channel][4 + page] > 0) encoders[current_channel][4 + page] -= 3;
      midi.sendControlChange({ 73 + page, myChannel[current_channel] }, encoders[current_channel][4 + page]);
    }
    aLastState4 = aState4;
    display.fillRect(24, 54, 36, 10, BLACK);
    display.fillRect(24, 54, map(encoders[current_channel][4 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }
  //_______________________________________________________Encoder5
  aState5 = mux.read(8);

  if (aState5 != aLastState5) {
    if (mux.read(9) != aState5) {
      if (encoders[current_channel][5 + page] <= 124) encoders[current_channel][5 + page] += 3;
      midi.sendControlChange({ 74 + page, myChannel[current_channel] }, encoders[current_channel][5 + page]);



    } else {
      if (encoders[current_channel][5 + page] > 0) encoders[current_channel][5 + page] -= 3;
      midi.sendControlChange({ 74 + page, myChannel[current_channel] }, encoders[current_channel][5 + page]);
    }
    aLastState5 = aState5;
    display.fillRect(91, 15, 36, 10, BLACK);
    display.fillRect(91, 15, map(encoders[current_channel][5 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }
  //_______________________________________________________Encoder6
  aState6 = mux.read(10);

  if (aState6 != aLastState6) {
    if (mux.read(11) != aState6) {
      if (encoders[current_channel][6 + page] <= 124) encoders[current_channel][6 + page] += 3;
      midi.sendControlChange({ 75 + page, myChannel[current_channel] }, encoders[current_channel][6 + page]);



    } else {
      if (encoders[current_channel][6 + page] > 0) encoders[current_channel][6 + page] -= 3;
      midi.sendControlChange({ 75 + page, myChannel[current_channel] }, encoders[current_channel][6 + page]);
    }
    aLastState6 = aState6;
    display.fillRect(91, 28, 36, 10, BLACK);
    display.fillRect(91, 28, map(encoders[current_channel][6 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }
  //_______________________________________________________Encoder7
  aState7 = mux.read(12);

  if (aState7 != aLastState7) {
    if (mux.read(13) != aState7) {
      if (encoders[current_channel][7 + page] <= 124) encoders[current_channel][7 + page] += 3;
      midi.sendControlChange({ 76 + page, myChannel[current_channel] }, encoders[current_channel][7 + page]);


    } else {
      if (encoders[current_channel][7 + page] > 0) encoders[current_channel][7 + page] -= 3;
      midi.sendControlChange({ 76 + page, myChannel[current_channel] }, encoders[current_channel][7 + page]);
    }
    aLastState7 = aState7;
    display.fillRect(91, 41, 36, 10, BLACK);
    display.fillRect(91, 41, map(encoders[current_channel][7 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }
  //_______________________________________________________Encoder8
  aState8 = mux.read(14);

  if (aState8 != aLastState8) {
    if (mux.read(15) != aState8) {
      if (encoders[current_channel][8 + page] <= 124) encoders[current_channel][8 + page] += 3;
      midi.sendControlChange({ 77 + page, myChannel[current_channel] }, encoders[current_channel][8 + page]);



    } else {
      if (encoders[current_channel][8 + page] > 0) encoders[current_channel][8 + page] -= 3;
      midi.sendControlChange({ 77 + page, myChannel[current_channel] }, encoders[current_channel][8 + page]);
    }
    aLastState8 = aState8;
    display.fillRect(91, 54, 36, 10, BLACK);
    display.fillRect(91, 54, map(encoders[current_channel][8 + page], 0, 127, 0, 37), 10, WHITE);
    refreshdisplay = 1;
  }

  //__________________________________________________________________________________________________________BUTTONS

  //_________________________________________________________1.Kick button
  if ((currtouched1 & _BV(0)) && !(lasttouched1 & _BV(0))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[1] = !mute[1];
        if (velocities[1] == 127) velocities[1] = 0;
        else velocities[1] = 127;
      } else {
        current_channel = 1;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
        updateDisplay();
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][1] = !sequencer[current_channel][steptrack[current_channel]][1];
    }
  }

  //________________________________________________________2. Snare buton
  if ((currtouched1 & _BV(1)) && !(lasttouched1 & _BV(1))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[2] = !mute[2];
        if (velocities[2] == 127) velocities[2] = 0;
        else velocities[2] = 127;
      } else {
        current_channel = 2;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][2] = !sequencer[current_channel][steptrack[current_channel]][2];
    }
    updateDisplay();
  }

  //_______________________________________________________3.clap button
  if ((currtouched1 & _BV(2)) && !(lasttouched1 & _BV(2))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[3] = !mute[3];
        if (velocities[3] == 127) velocities[3] = 0;
        else velocities[3] = 127;
      } else {
        current_channel = 3;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][3] = !sequencer[current_channel][steptrack[current_channel]][3];
    }
    updateDisplay();
  }

  //_________________________________________________________4.CH button
  if ((currtouched1 & _BV(3)) && !(lasttouched1 & _BV(3))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[4] = !mute[4];
        if (velocities[4] == 127) velocities[4] = 0;
        else velocities[4] = 127;
      } else {
        current_channel = 4;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][4] = !sequencer[current_channel][steptrack[current_channel]][4];
    }
    updateDisplay();
  }

  //__________________________________________________________5.OH button
  if ((currtouched1 & _BV(4)) && !(lasttouched1 & _BV(4))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[5] = !mute[5];
        if (velocities[5] == 127) velocities[5] = 0;
        else velocities[5] = 127;
      } else {
        current_channel = 5;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][5] = !sequencer[current_channel][steptrack[current_channel]][5];
    }
    updateDisplay();
  }

  //___________________________________________________________6.Perc1 button
  if ((currtouched1 & _BV(5)) && !(lasttouched1 & _BV(5))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[6] = !mute[6];
        if (velocities[6] == 127) velocities[6] = 0;
        else velocities[6] = 127;
      } else {
        current_channel = 6;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][6] = !sequencer[current_channel][steptrack[current_channel]][6];
    }
    updateDisplay();
  }

  //___________________________________________________________7.Perc2 button
  if ((currtouched1 & _BV(6)) && !(lasttouched1 & _BV(6))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[7] = !mute[7];
        if (velocities[7] == 127) velocities[7] = 0;
        else velocities[7] = 127;
      } else {
        current_channel = 7;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][7] = !sequencer[current_channel][steptrack[current_channel]][7];
    }
    updateDisplay();
  }

  //__________________________________________________________8.Perc3 button
  if ((currtouched1 & _BV(7)) && !(lasttouched1 & _BV(7))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[8] = !mute[8];
        if (velocities[8] == 127) velocities[8] = 0;
        else velocities[8] = 127;
      } else {
        current_channel = 8;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][8] = !sequencer[current_channel][steptrack[current_channel]][8];
    }
    updateDisplay();
  }

  //___________________________________________________________9.Perc4 button
  if ((currtouched1 & _BV(8)) && !(lasttouched1 & _BV(8))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[9] = !mute[9];
        if (velocities[9] == 127) velocities[9] = 0;
        else velocities[9] = 127;
      } else {
        current_channel = 9;
        midi.sendNoteOn(Notes[current_channel], 127);
        midi.sendNoteOff(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][9] = !sequencer[current_channel][steptrack[current_channel]][9];
    }
    updateDisplay();
  }

  //___________________________________________________________10.Atmos button
  if ((currtouched1 & _BV(9)) && !(lasttouched1 & _BV(9))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[10] = !mute[10];
        if (velocities[10] == 127) velocities[10] = 0;
        else velocities[10] = 127;
      } else {
        current_channel = 10;
        midi.sendNoteOn(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][10] = !sequencer[current_channel][steptrack[current_channel]][10];
    }
    updateDisplay();
  }

  //___________________________________________________________11.Melody2 button
  if ((currtouched1 & _BV(10)) && !(lasttouched1 & _BV(10))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[11] = !mute[11];
        if (velocities[11] == 127) velocities[11] = 0;
        else velocities[11] = 127;
      } else {
        current_channel = 11;
        midi.sendNoteOn(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][11] = !sequencer[current_channel][steptrack[current_channel]][11];
    }
    updateDisplay();
  }

  //___________________________________________________________12. Melody3 button
  if ((currtouched2 & _BV(9)) && !(lasttouched2 & _BV(9))) {
    if (!stepstate) {
      button4 = false;
      if (mutestate) {
        mute[12] = !mute[12];
        if (velocities[12] == 127) velocities[12] = 0;
        else velocities[12] = 127;
      } else {
        current_channel = 12;
        midi.sendNoteOn(Notes[current_channel], 127);
      }
    } else {
      sequencer[current_channel][steptrack[current_channel]][12] = !sequencer[current_channel][steptrack[current_channel]][12];
    }
    updateDisplay();
  }

  //___________________________________________________________13. 13 button
  if ((currtouched2 & _BV(0)) && !(lasttouched2 & _BV(0))) {
    if (stepstate) sequencer[current_channel][steptrack[current_channel]][13] = !sequencer[current_channel][steptrack[current_channel]][13];
  }

  //___________________________________________________________14. 14 button
  if ((currtouched2 & _BV(1)) && !(lasttouched2 & _BV(1))) {
    if (stepstate) sequencer[current_channel][steptrack[current_channel]][14] = !sequencer[current_channel][steptrack[current_channel]][14];
  }


  //___________________________________________________________15. 15 button
  if ((currtouched2 & _BV(2)) && !(lasttouched2 & _BV(2))) {
    if (stepstate) sequencer[current_channel][steptrack[current_channel]][15] = !sequencer[current_channel][steptrack[current_channel]][15];
  }


  //___________________________________________________________16. 16 button
  if ((currtouched2 & _BV(3)) && !(lasttouched2 & _BV(3))) {
    if (!stepstate) {
      button4 = true;
      global = !global;
      display.clearDisplay();
      display.setCursor(35, 32);
      display.print("Tempo: ");
      display.print(tempo);
      refreshdisplay = 1;
    } else
      sequencer[current_channel][steptrack[current_channel]][16] = !sequencer[current_channel][steptrack[current_channel]][16];
  }

  //________________________________________________On/Off button
  if ((currtouched2 & _BV(5)) && !(lasttouched2 & _BV(5))) {
    onoffstate = !onoffstate;
    if (!onoffstate) {
      tick_count = 0;
      current_led = 16;
      current_step = 1;
      for (int j = 1; j < 11; j++) track[j] = 4;
      midi.sendStop();
      midi.sendStop();
    } else {
      midi.sendStart();
      last_tick = micros();
    }
  }

  //_________________________________________________________Step Button
  if ((currtouched2 & _BV(4)) && !(lasttouched2 & _BV(4))) {
    stepstate = !stepstate;
    mutestate = false;
    steptrack[current_channel] = track[current_channel];
  }

  //__________________________________________________________Mute Button
  if ((currtouched2 & _BV(6)) && !(lasttouched2 & _BV(6))) {
    mutestate = !mutestate;
  }

  //_____________________________________________________________Chain Button
  if ((currtouched2 & _BV(7)) && !(lasttouched2 & _BV(7))) {
    chainstate[current_channel] = !chainstate[current_channel];
  }

  //__________________________________________________________Track button
  if ((currtouched2 & _BV(8)) && !(lasttouched2 & _BV(8)))
    if (!stepstate) {
      track[current_channel]--;
      if (track[current_channel] == 0) track[current_channel] = 4;
    } else {
      steptrack[current_channel]--;
      if (steptrack[current_channel] == 0) steptrack[current_channel] = 4;
    }


  //________________________________Menu encoder 1
  aState = digitalRead(CLK);
  if (aState != aLastState) {
    if (digitalRead(DT) != aState) {
      if (button4) {
        tempo++;
        us_per_tick = (unsigned long)(1e6 / (tempo * 24 / 60));

        display.clearDisplay();
        display.setCursor(35, 32);
        display.print("Tempo: ");
        display.print(tempo);
        refreshdisplay = 1;
      } else {
        page = page + 8;
        if (page == 24) page = 0;
        updateDisplay();
      }

    } else {
      if (button4) {
        tempo--;
        us_per_tick = (unsigned long)(1e6 / (tempo * 24 / 60));

        display.clearDisplay();
        display.setCursor(35, 32);
        display.print("Tempo: ");
        display.print(tempo);
        refreshdisplay = 1;
      } else {
        page = page - 8;
        if (page == -8) page = 16;
        updateDisplay();
      }
    }
  }
  aLastState = aState;

  //__________Refresh OLED
  if (refreshdisplay == 1)
    if (millis() - lastoled >= oledtime) {
      lastoled = millis();
      refreshdisplay = 0;
      display.display();
    }

  lasttouched1 = currtouched1;
  lasttouched2 = currtouched2;
}

void updateDisplay() {

  display.clearDisplay();
  display.setCursor(0, 1);
  display.print(text[current_channel]);

  display.drawLine(0, 11, 128, 11, WHITE);

  if (page == 0) {
    display.setTextColor(BLACK);
    display.fillRect(68, 0, 13, 10, WHITE);
    display.setCursor(72, 1);
    display.print("1");
    display.setTextColor(WHITE);
    display.setCursor(89, 1);
    display.print("2");
    display.setCursor(106, 1);
    display.print("LFO");

    display.setCursor(0, 17);
    display.println("SAMPLE ");
    display.setCursor(46, 17);
    display.print(Note[current_channel] - 35);

    display.setCursor(0, 30);
    display.println("VOL");
    display.fillRect(23, 28, 1, 10, WHITE);
    display.fillRect(60, 28, 1, 10, WHITE);
    display.fillRect(24, 28, map(encoders[current_channel][2 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 43);
    display.println("TON");
    display.fillRect(23, 41, 1, 10, WHITE);
    display.fillRect(60, 41, 1, 10, WHITE);
    display.fillRect(24, 41, map(encoders[current_channel][3 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 56);
    display.println("HIG");
    display.fillRect(23, 54, 1, 10, WHITE);
    display.fillRect(60, 54, 1, 10, WHITE);
    display.fillRect(24, 54, map(encoders[current_channel][4 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 17);
    display.println("MID");
    display.fillRect(90, 15, 1, 10, WHITE);
    display.fillRect(127, 15, 1, 10, WHITE);
    display.fillRect(91, 15, map(encoders[current_channel][5 + page], 0, 127, 0, 37), 10, WHITE);


    display.setCursor(67, 30);
    display.println("LOW");
    display.fillRect(90, 28, 1, 10, WHITE);
    display.fillRect(127, 28, 1, 10, WHITE);
    display.fillRect(91, 28, map(encoders[current_channel][6 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 43);
    display.println("ATT");
    display.fillRect(90, 41, 1, 10, WHITE);
    display.fillRect(127, 41, 1, 10, WHITE);
    display.fillRect(91, 41, map(encoders[current_channel][7 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 56);
    display.println("REL");
    display.fillRect(90, 54, 1, 10, WHITE);
    display.fillRect(127, 54, 1, 10, WHITE);
    display.fillRect(91, 54, map(encoders[current_channel][8 + page], 0, 127, 0, 37), 10, WHITE);


  } else if (page == 8) {
    display.setTextColor(WHITE);
    display.setCursor(72, 1);
    display.print("1");
    display.setCursor(106, 1);
    display.print("LFO");
    display.setTextColor(BLACK);
    display.fillRect(85, 0, 13, 10, WHITE);
    display.setCursor(89, 1);
    display.print("2");


    display.setTextColor(WHITE);
    display.setCursor(0, 17);
    display.println("RV1");
    display.fillRect(23, 15, 1, 10, WHITE);
    display.fillRect(60, 15, 1, 10, WHITE);
    display.fillRect(24, 15, map(encoders[current_channel][1 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 30);
    display.println("RV2");
    display.fillRect(23, 28, 1, 10, WHITE);
    display.fillRect(60, 28, 1, 10, WHITE);
    display.fillRect(24, 28, map(encoders[current_channel][2 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 43);
    display.println("RV3");
    display.fillRect(23, 41, 1, 10, WHITE);
    display.fillRect(60, 41, 1, 10, WHITE);
    display.fillRect(24, 41, map(encoders[current_channel][3 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 56);
    display.println("DL1");
    display.fillRect(23, 54, 1, 10, WHITE);
    display.fillRect(60, 54, 1, 10, WHITE);
    display.fillRect(24, 54, map(encoders[current_channel][4 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 17);
    display.println("DL2");
    display.fillRect(90, 15, 1, 10, WHITE);
    display.fillRect(127, 15, 1, 10, WHITE);
    display.fillRect(91, 15, map(encoders[current_channel][5 + page], 0, 127, 0, 37), 10, WHITE);


    display.setCursor(67, 30);
    display.println("DL3");
    display.fillRect(90, 28, 1, 10, WHITE);
    display.fillRect(127, 28, 1, 10, WHITE);
    display.fillRect(91, 28, map(encoders[current_channel][6 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 43);
    display.println("SAT");
    display.fillRect(90, 41, 1, 10, WHITE);
    display.fillRect(127, 41, 1, 10, WHITE);
    display.fillRect(91, 41, map(encoders[current_channel][7 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 56);
    display.println("STE");
    display.fillRect(90, 54, 1, 10, WHITE);
    display.fillRect(127, 54, 1, 10, WHITE);
    display.fillRect(91, 54, map(encoders[current_channel][8 + page], 0, 127, 0, 37), 10, WHITE);
  } else {
    display.setTextColor(WHITE);
    display.setCursor(72, 1);
    display.print("1");
    display.setCursor(89, 1);
    display.print("2");
    display.fillRect(102, 0, 24, 10, WHITE);
    display.setCursor(106, 1);
    display.setTextColor(BLACK);
    display.print("LFO");

    display.setTextColor(WHITE);
    display.setCursor(0, 17);
    display.println("P-R");
    display.fillRect(23, 15, 1, 10, WHITE);
    display.fillRect(60, 15, 1, 10, WHITE);
    display.fillRect(24, 15, map(encoders[current_channel][1 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 30);
    display.println("P-D");
    display.fillRect(23, 28, 1, 10, WHITE);
    display.fillRect(60, 28, 1, 10, WHITE);
    display.fillRect(24, 28, map(encoders[current_channel][2 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 43);
    display.println("V-R");
    display.fillRect(23, 41, 1, 10, WHITE);
    display.fillRect(60, 41, 1, 10, WHITE);
    display.fillRect(24, 41, map(encoders[current_channel][3 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(0, 56);
    display.println("V-D");
    display.fillRect(23, 54, 1, 10, WHITE);
    display.fillRect(60, 54, 1, 10, WHITE);
    display.fillRect(24, 54, map(encoders[current_channel][4 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 17);
    display.println("R-R");
    display.fillRect(90, 15, 1, 10, WHITE);
    display.fillRect(127, 15, 1, 10, WHITE);
    display.fillRect(91, 15, map(encoders[current_channel][5 + page], 0, 127, 0, 37), 10, WHITE);


    display.setCursor(67, 30);
    display.println("R-D");
    display.fillRect(90, 28, 1, 10, WHITE);
    display.fillRect(127, 28, 1, 10, WHITE);
    display.fillRect(91, 28, map(encoders[current_channel][6 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 43);
    display.println("D-R");
    display.fillRect(90, 41, 1, 10, WHITE);
    display.fillRect(127, 41, 1, 10, WHITE);
    display.fillRect(91, 41, map(encoders[current_channel][7 + page], 0, 127, 0, 37), 10, WHITE);

    display.setCursor(67, 56);
    display.println("D-D");
    display.fillRect(90, 54, 1, 10, WHITE);
    display.fillRect(127, 54, 1, 10, WHITE);
    display.fillRect(91, 54, map(encoders[current_channel][8 + page], 0, 127, 0, 37), 10, WHITE);
  }

  refreshdisplay = 1;
}
