#include <cmath>
#include <iostream>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

#define WIDTH 1000
#define HEIGHT 1000
#define TYPE bool

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
  for(int i=1;i<=n;i++) {
    step(world, buffer);

    TYPE** swap = world;
    world = buffer;
    buffer = swap;

    //cout << "step " << i << " world: " << world << " buffer:" << buffer << endl;
  }

  //print(world);

  return world;
}

int main() {
  signal(SIGSEGV, handler);

  TYPE** world = initZeroWorld();
  initRPentomino(world);
  world = step(1104, world);
}
