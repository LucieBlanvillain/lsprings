#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

struct fmatrix
{
  float *matrix;
  int h, w;
};

struct fresizablematrix open_csv(const char *filename) {

}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s [file]", argv[1]);
  }
}
