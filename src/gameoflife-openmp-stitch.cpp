#include <cmath>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <GL/glut.h>

#define SIZE 13000
#define WIDTH SIZE
#define HEIGHT SIZE
#define TYPE bool
#define bigint long
#define LINESIZE 20000
#define CR 13
#define LF 10
#define BUFFSIZE 8192

using namespace std;

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

TYPE** initZeroWorld() {
  TYPE** world = new TYPE*[WIDTH];

  for (int i = 0; i < WIDTH; i++) {
    world[i] = new TYPE[HEIGHT];
    for (int j = 0; j < HEIGHT; j++) {
      world[i][j] = 0;
    }
  }

  return world;
}

void initRPentomino(TYPE** world) {
  int x = WIDTH / 2 - 2;
  int y = HEIGHT / 2 - 2;
  //   012
  // 0  ##
  // 1 ##
  // 2  #
  world[x + 0][y + 0] = false;
  world[x + 1][y + 0] = true;
  world[x + 2][y + 0] = true;
  world[x + 0][y + 1] = true;
  world[x + 1][y + 1] = true;
  world[x + 2][y + 1] = false;
  world[x + 0][y + 2] = false;
  world[x + 1][y + 2] = true;
  world[x + 2][y + 2] = false;
}

void initFromFile(TYPE** world, char* path) {
  // read file
  struct stat sb;
  if (stat(path, &sb) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  char* cinput = new char[sb.st_size];
  FILE* fh = fopen(path, "r");
  fread(cinput, sb.st_size, 1, fh);
  fclose(fh);

  // split into tokens
  vector<string> tokens;
  string input = cinput;
  string token;
  int beginToken = 0;
  for (unsigned int i = 0; i < input.length(); i++) {
    if (input[i] == 'o' || input[i] == 'b' || input[i] == '$') {
      token = input.substr(beginToken, i - beginToken + 1);
      tokens.push_back(token);
      //cout << "found token at " << i << ": " << input[i] << " (1) " << token
      //    << endl;
      beginToken = i + 1;
    } else if (input[i] == '\n') {
      beginToken = i + 1;
    }
  }
  string lastToken = input.substr(beginToken, input.length() - beginToken + 1);
  if (lastToken != "")
    tokens.push_back(lastToken);
  //cout << "last token: " << lastToken << endl;

  // process the tokens
  vector<string>::iterator t;
  int x = 0, y = 0; // the cursor
  for (t = tokens.begin(); t != tokens.end(); ++t) {
    char symbol = (*t).at((*t).size() - 1);
    int num = 1;
    if ((*t).length() > 1) {
      num = (int) strtol((*t).substr(0, (*t).size() - 1).c_str(), NULL, 10);
    }
    switch (symbol) {
    case 'b':
      for (int i = 0; i < num; i++)
        world[x + i][y] = false;
      x += num;
      break;
    case 'o':
      for (int i = 0; i < num; i++)
        world[x + i][y] = true;
      x += num;
      break;
    case '$':
      y += num;
      x = 0;
      break;
    }
  }
}

void print(TYPE** world) {
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      cout << (world[j][i] ? '#' : '.');
    }
    cout << endl;
  }
  cout << "=====================" << endl;
}

int countNeighbourhoodAroundTheCorner(TYPE** world, int x, int y) {
  // avoid x < 2 or y < 2
  x += WIDTH;
  y += HEIGHT;
  return world[(x + 0) % WIDTH][(y - 1) % HEIGHT] // top
  + world[(x + 1) % WIDTH][(y - 1) % HEIGHT] // top right
      + world[(x + 1) % WIDTH][(y - 0) % HEIGHT] // right
      + world[(x + 1) % WIDTH][(y + 1) % HEIGHT] // bottom right
      + world[(x + 0) % WIDTH][(y + 1) % HEIGHT] // bottom
      + world[(x - 1) % WIDTH][(y + 1) % HEIGHT] // bottom left
      + world[(x - 1) % WIDTH][(y - 0) % HEIGHT] // left
      + world[(x - 1) % WIDTH][(y - 1) % HEIGHT]; // top left
}

int countNeighbourhood(TYPE** world, int x, int y) {
  return countNeighbourhoodAroundTheCorner(world, x, y);
}

void step(TYPE** world, TYPE** buffer) {
#pragma omp for
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      int n = countNeighbourhood(world, i, j);
      buffer[i][j] = world[i][j] ? (n == 2 || n == 3) : n == 3;
    }
  }
}

TYPE** step(int n, TYPE** world) {
  TYPE** buffer = initZeroWorld();

  //print(world);

#pragma omp parallel
  for (int i = 1; i <= n; i++) {
    step(world, buffer);

    TYPE** swap = world;
    world = buffer;
    buffer = swap;

    //cout << "step " << i << " world: " << world << " buffer:" << buffer << endl;
  }

  //print(world);

  return world;
}

TYPE** gworld;
unsigned char texture[WIDTH][HEIGHT][3];

void renderScene() {

  int black = 0, white = 0;
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      bool expr = gworld[j][WIDTH - 1 - i];
      texture[i][j][0] = expr ? 255 : 0;
      texture[i][j][1] = expr ? 255 : 0;
      texture[i][j][2] = expr ? 255 : 0;
      expr ? white++ : black++;
    }
  }

  cout << "White: " << white << " Black: " << black << endl;

  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB,
      GL_UNSIGNED_BYTE, &texture[0][0][0]);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(-1.0, -1.0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(1.0, -1.0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex2f(1.0, 1.0);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(-1.0, 1.0);
  glEnd();

  glFlush();
  glutSwapBuffers();
}

float zoom = 10;

void processMouse(int button, int state, int x, int y) {
  if (state == 0) {
    if (button == 3) {
      cout << "Zoom in: " << --zoom << endl;
    }
    if (button == 4) {
      cout << "Zoom out: " << ++zoom << endl;
    }
    glOrtho(-zoom/10, zoom/10, -zoom/10, zoom/10, -10, 10);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene();
    glFlush();
  }
}

void processKeyboard(unsigned char key, int x, int y) {
  cout << "key: " << key << endl;
}

int main(int argc, char** argv) {
  signal(SIGSEGV, handler);

  gworld = initZeroWorld();
  initFromFile(gworld, "../utm.lrle");
  //print(gworld);

  /*glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(500, 500);
  glutCreateWindow("game of life openmp");
  glutMouseFunc(processMouse);
  glutKeyboardFunc(processKeyboard);

  glutDisplayFunc(renderScene);

  glutMainLoop();*/

  step(10000, gworld);

  return 0;
}

