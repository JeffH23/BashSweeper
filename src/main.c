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
#include <termios.h>
#include <poll.h>
#include <time.h>

//signal handling
volatile sig_atomic_t windowResized = 0;
void handle_sigwinch(int sig){
  windowResized = 1;
}
struct termios defaultTerm;
void resetTerm(int sig){
  int termResetStatus = tcsetattr(STDIN_FILENO, TCSANOW, &defaultTerm);
  _exit(1);
}
//Function stubs
int colorCycle(int *color, float frame);
void intTo7Seg(int num);
int constructGameBoard(int gameMap[], int gameMapLen);
void findNeighbor(int gameMap[], int gameMapLen, int *index, int *neighbors);
char handleInput(struct pollfd *fdToPoll, int cursorPosition[]);
void drawGame(int gameMap[], int mapSize, int sevenSegTimerTime, int flagCounter, int cursorPosition[], char instruction);

int main(){
  signal(SIGINT, resetTerm);
  signal(SIGSEGV, resetTerm);

  int getDefaultAttributes = tcgetattr(STDIN_FILENO, &defaultTerm);
  struct termios newTermSettings = defaultTerm;
  newTermSettings.c_lflag &= ~(ICANON | ECHO);
  int setNewAttributes = tcsetattr(STDIN_FILENO, TCSANOW, &newTermSettings);
  struct pollfd fdToPoll;
  fdToPoll.fd = STDIN_FILENO;
  fdToPoll.events = POLLIN;

/*  char positionBuff[16];
  printf("\033[2J");
  fflush(NULL);
  for(int i = 0; i < 20; i++){
    for(int j = 0; j < 20; j++){
      usleep(20000);
      if(write(STDOUT_FILENO, positionBuff, snprintf(positionBuff, 16, "\033[%d;%dH", i, j)) == -1){
        printf("error");
      }
    }
  }*/

  int mapSize = 81;
  int gameMap[81] = {0};
  int gameBoardStatus = constructGameBoard(gameMap,81);
  int cursorPosition[2] = {1,1};
  int sevenSegTimerTime = 0;
  int flagCounter = 10;
  time_t startTime;
  time(&startTime);
  time_t lastUpdateTime;
  time(&lastUpdateTime);
/*
  intTo7Seg(-1);
  intTo7Seg(-10);
  intTo7Seg(-98);
  intTo7Seg(1);
  intTo7Seg(10);
  intTo7Seg(100);*/
  bool gameRunning = 1;
  while(gameRunning){
    int drawFlag = 0;
    time_t currentTime;
    time(&currentTime);
    sevenSegTimerTime = -1;
    if(currentTime - lastUpdateTime != 0){
      lastUpdateTime = currentTime;
      sevenSegTimerTime = currentTime - startTime;
      drawFlag = 1;
    }
    char input = 'n';
    if(poll(&fdToPoll, 1, 0) > 0){
      handleInput(&fdToPoll, cursorPosition);
      drawFlag = 1;
    }
    if(drawFlag){
      drawGame(gameMap, mapSize, sevenSegTimerTime, flagCounter, cursorPosition, input);
    }
  }
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
  int n = sprintf(numString,"%d", num);
  for(int i = 0; i < n; i++){
    memmove(charBuff + 4, charBuff, 8);//pushes the first 2 characters toward the end of the array

    switch(numString[i]){//populates the zeroth index of the array with the apropriate unicode character
      case '-':
        strncpy(charBuff, "\U0001FB07)", 4);
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
  
  char tempBuff[4];
  strncpy(tempBuff, charBuff + 8, 4);
  memmove(charBuff + 8, charBuff, 4);
  strncpy(charBuff, tempBuff, 4);
  printf("%s",charBuff);
  fflush(NULL);
}

int constructGameBoard(int gameMap[], int gameMapLen){ int NumRequestedMines = 10;
  int MinePositions[NumRequestedMines];
  for(int i = 0; i < NumRequestedMines; i++){//set all MinePositions to -1
    MinePositions[i] = -1;
  }
  for(int i = 0; i < NumRequestedMines;){//sets the positions of the mines and check for duplicates
    int newMinePosition = rand() % gameMapLen;
    for(int j = 0; j < NumRequestedMines; j++){//checks all MinePositions and breaks on match or end of set positions
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
  int neighbors[8];
  memset(neighbors, 0xFF, 8 * sizeof(int));
  for(int i = 0; i < NumRequestedMines; i++){//set the mine ajacent cell's numbers
    findNeighbor(gameMap, gameMapLen, &MinePositions[i], neighbors);
    for(int j = 0; j < 8; j++){
      if(neighbors[j] > -1){
        gameMap[neighbors[j]]++;
      }
    }
  }
  /*
  for(int i = 0; i < 9; i++){
    for(int j = 0; j < 9; j++){
      printf("%d", gameMap[i * 9 + j]);
    }
    printf("\n");
  };*/
  return 0;
}

void findNeighbor(int gameMap[], int gameMapLen, int *index, int *neighbors){
  int boardWidth = sqrt(gameMapLen);
  if(*index < boardWidth){//top row
    if(*index == 0){//sort for top left corner
      neighbors[0] = 1;
      neighbors[1] = boardWidth + 1;
      neighbors[2] = boardWidth;
    }
    if(*index == boardWidth - 1){//sort for top right corner
      neighbors[0] = boardWidth - 2;
      neighbors[1] = boardWidth + boardWidth - 2;
      neighbors[2] = boardWidth + boardWidth - 1;
    }
    else{//sort for top row excluding corners
      neighbors[0] = *index + 1;
      neighbors[1] = *index + boardWidth + 1;
      neighbors[2] = *index + boardWidth;
      neighbors[3] = *index + boardWidth - 1;
      neighbors[4] = *index - 1;
    }
  }
  if(*index >= gameMapLen - boardWidth){// bottom row
    if(*index == gameMapLen - boardWidth){//sort for bottom left corner
      neighbors[0] = *index - boardWidth;
      neighbors[1] = *index - boardWidth + 1;
      neighbors[2] = *index + 1;
    }
    if(*index == gameMapLen - 1){// sort for bottom right corner
      neighbors[0] = *index - boardWidth - 1;
      neighbors[1] = *index - boardWidth;
      neighbors[2] = *index - 1;
    }
    else{//sort for bottom row excluding corners
      neighbors[0] = *index - boardWidth - 2;
      neighbors[1] = *index - boardWidth - 1;
      neighbors[2] = *index - boardWidth;
      neighbors[3] = *index + 1;
      neighbors[4] = *index - 1;
    }
  }
  else{
    if(*index % boardWidth < 1){// sort for left side excluding corners
      neighbors[0] = *index - boardWidth;
      neighbors[1] = *index - boardWidth + 1;
      neighbors[2] = *index + 1;
      neighbors[3] = *index + boardWidth + 1;
      neighbors[4] = *index + boardWidth;
    }
    if(*index % boardWidth == boardWidth - 1){//sort for right side excluding corners
      neighbors[0] = *index - boardWidth - 1;
      neighbors[1] = *index - boardWidth;
      neighbors[4] = *index + boardWidth;
      neighbors[3] = *index + boardWidth - 1;
      neighbors[2] = *index - 1;
  }
    else{//all the middle ones
      neighbors[0] = *index - boardWidth - 1;
      neighbors[1] = *index - boardWidth;
      neighbors[2] = *index - boardWidth + 1;
      neighbors[3] = *index + 1;
      neighbors[4] = *index + boardWidth + 1;
      neighbors[5] = *index + boardWidth;
      neighbors[6] = *index + boardWidth - 1;
      neighbors[7] = *index - 1;
    }
  }
}
char handleInput(struct pollfd *fdToPoll, int cursorPosition[]){
  //read from STDIN
  if(fdToPoll->revents == POLLIN){//there is data to read
    int dataAvalible = 1;
    int data;
    while(dataAvalible){
      data = getchar();
      int pollStatus = poll(fdToPoll, 1, 0);
      if(fdToPoll->revents != POLLIN){
        dataAvalible = 0;
      }
    }
    char commandedPosition[16];
    switch(data){
      case 'h':
        if(cursorPosition[1]>1){cursorPosition[1]--;}
        if(write(STDOUT_FILENO,commandedPosition, snprintf(commandedPosition, 16, "\033[%dG", cursorPosition[1])) == -1){
          printf("Error,handleInput");
        }
        return 'n';
        break;
      case 'k':
        if(cursorPosition[0]>1){cursorPosition[0]--;}
        if(write(STDOUT_FILENO,commandedPosition, snprintf(commandedPosition, 16, "\033[%d;%dH",cursorPosition[0], cursorPosition[1])) == -1){
          printf("Error,handleInput");
        }
        return 'n';
        break;
      case 'l':
        if(cursorPosition[1]<20){cursorPosition[1]++;}
        if(write(STDOUT_FILENO,commandedPosition, snprintf(commandedPosition, 16, "\033[%dG", cursorPosition[1])) == -1){
          printf("Error,handleInput");
        }
        return 'n';
        break;
      case 'j':
        if(cursorPosition[0]<20){cursorPosition[0]++;}
        if(write(STDOUT_FILENO,commandedPosition, snprintf(commandedPosition, 16, "\033[%d;%dH",cursorPosition[0], cursorPosition[1])) == -1){
          printf("Error,handleInput");
        }
        return 'n';
        break;
      case 'e':
        printf("exit\n");
        raise(SIGINT);
        break;
      case 'f':
        return 'f';
        break;
      default:
        return 'n';
    }
  }
}

void drawGame(int gameMap[], int mapSize, int sevenSegTimerTime, int flagCounter, int cursorPosition[], char instruction){
  printf("\033[?25l");
  printf("\033[1;1H🬹🬹🬹🬹🬹🬹🬹🬹🬹🬹🬹🬹🬹");
  printf("\033[2;1H█");
  intTo7Seg(flagCounter);
  printf("\033[2;5H▐:)█");
  //printf("\033[3;1H");

  intTo7Seg(sevenSegTimerTime);
  printf("\033[2;12H▐█");
  printf("\033[3;1H█████████████");
  fflush(NULL);
  char commandedPosition[16];//reset cursor position
  if(write(STDOUT_FILENO,commandedPosition, snprintf(commandedPosition, 16, "\033[%d;%dH",cursorPosition[0], cursorPosition[1])) == -1){
    printf("Error,handleInput");
  }
  printf("\033[?25h");//unhide Cursor

  if(instruction == 'n'){
    return;
  }
  if(instruction == 'f'){
    printf("flag");
  }
}
