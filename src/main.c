//⚑■
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

//signal handling
volatile sig_atomic_t windowResized = 0;
void handle_sigwinch(int sig){
  windowResized = 1;
}

//Function stubs
int colorCycle(int *color, float frame);
void intTo7Seg(int num);
int constructGameBoard(int *gameMap, int gameMapLen);

int main(){
  const char *CURSOR_CLEAR = "\033[0k";
  const char *CURSOR_START = "\033[2J\033[H";
  const char *RESET = "\033[0m";
  int cursorOffset[2] = {0,0};
  int color[] = {0,0,0};
  printf("\033[?25l");//hides cursor
  printf("\033[?25h");//show cursor
  struct winsize windowSize;//struct to hold queried window windowSize
  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize) == -1){
    perror("ioctl TIOCGWINSZ failed");
    return -1;
  }
  printf("\U0001FBF9\U0001FBF9\U0001FBF9");

  printf("columns: %d\n", windowSize.ws_col);
  if(windowSize.ws_col >= windowSize.ws_row){
    //setoffset
  }
/*
  bool gameRunning = 1;
  while(gameRunning){
    if(windowResized){
      if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize) == -1){
        perror("ioctl TIOCGWINSZ failed");
        break;
      }
      printf("%d", windowSize.ws_col);
    }
  }*/
  int testnum = 284;
  intTo7Seg(testnum);
//  printf("\033[9999;9999H");

  int mapSize = 81;
  int gameMap[81];
  int gameBoardStatus = constructGameBoard(gameMap,81);
  return 0;
}

int colorCycle(int *color, float frame){
  float x = frame * .01;
  int e = 1; //constant to adjust clamping factor
  color[0] = (.5*(sqrt(pow(sin(x), 2.0) + e) / sqrt(1+e)) + (sin(x) / 2.0)) * 255;
  color[1] = (.5*(sqrt(pow(sin(x + ((5.0/3.0) * 3.14159)), 2.0) + e)/sqrt(1+e)) + (sin(x + ((2.0/3.0) * 3.14159)) / 2.0)) * 255;
  color[2] = (.5*(sqrt(pow(sin(x + ((7.0/3.0) * 3.14159)), 2.0) + e)/sqrt(1+e)) + (sin(x + ((4.0/3.0) * 3.14159)) / 2.0)) * 255;
  return 0;
}

void intTo7Seg(int num){
  if(num < -99){
    printf("\U0001FB79\U0001FBF9\U0001FBF9");
    return;
  }
  if(num > 999){
    printf("\U0001FBF9\U0001FBF9\U0001FBF9");
    return;
  }
  char numString[6];
  char charBuff[13] = {"\U0001FBF0\U0001FBF0\U0001FBF0"};
  sprintf(numString,"%d", num);
  for(int i = 3; i > 0; i--){
    memmove(charBuff + 4, charBuff, 8);//pushes the first 2 characters toward the end of the array

    switch(numString[i - 1]){//populates the zeroth index of the array with the apropriate unicode character
      case '-':
        strncpy(charBuff, "\U0001FB0B", 4);
        break;
      case '0':
        strncpy(charBuff, "\U0001FBF0", 4);
        break;
      case '1':
        strncpy(charBuff, "\U0001FBF1", 4);
        break;
      case '2':
        strncpy(charBuff, "\U0001FBF2", 4);
        break;
      case '3':
        strncpy(charBuff, "\U0001FBF3", 4);
        break;
      case '4':
        strncpy(charBuff, "\U0001FBF4", 4);
        break;
      case '5':
        strncpy(charBuff, "\U0001FBF5", 4);
        break;
      case '6':
        strncpy(charBuff, "\U0001FBF6", 4);
        break;
      case '7':
        strncpy(charBuff, "\U0001FBF7", 4);
        break;
      case '8':
        strncpy(charBuff, "\U0001FBF8", 4);
        break;
      case '9':
        strncpy(charBuff, "\U0001FBF9", 4);
        break;
    }
  }
  printf("%s\n",charBuff);
}

int constructGameBoard(int *gameMap, int gameMapLen){
  printf("constructGameBoard");
  int NumRequestedMines = 10;
  int NumPlacedMines = 0;
  int *MinePositions = malloc(NumRequestedMines * sizeof(int));
  if(*MinePositions == -1){//malloc safety check
    return -1;
  }
  for(int i = 0; i < 10; i++){//set all MinePositions to -1
    MinePositions[i] = -1;
  }
  for(int i = 0; i < 10;){//sets the positions of the mines and check for duplicates
    int newMinePosition = rand() % gameMapLen;
    for(int j = 0; j < 10; j++){//checks all MinePositions and breaks on match or end of set positions
      if(MinePositions[j] == -1){
        MinePositions[i] = newMinePosition;
        i++;
        break;
      }
      if(MinePositions[j] == newMinePosition){
        break;
      }
    }
  }
  for(int i = 0; i < NumRequestedMines; i++){
    printf("%d ", MinePositions[i]);
  }
  for(int i = 0; i < gameMapLen - 1; i++){
    //set each cell
  }
  return 0;
}
//  for (int i = 0; i < 10000; i++){
//    usleep(10000);
//    printf("%s", CURSOR_START);
//    int cc = colorCycle(color, (float)i);
//
//    int xPos = rand() % 20,yPos = rand() % 20;//make a random xy cordinate
//    printf("\033[%i;%iH", xPos, yPos);//place cursor
//    printf("\33[38;2;%d;%d;%dm\U0001FBF8", color[0], color[1], color[2]);
//
//    printf("\033[%i;%iH", 20, 0);//place cursor
//    printf("\33[38;2;%d;%d;%dm⚑  \n", color[0], color[1], color[2]);
//    printf("%3i, %3i, %3i frame:%i",color[0], color[1], color[2], i);
//
//    fflush(stdout);
//  }
//  printf("\033[?25h");//show cursor
//  printf("%s \n", RESET);
//  while (true){
//    printf("%s", CURSOR_START);
//
//  }

