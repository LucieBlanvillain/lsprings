#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <time.h>

#define NLOOPS 5
#define OBJECT_DIAMETER 30

struct fmatrix
{
  float *matrix;
  int h, w;
};

struct sim_state {
  int is_running;
  int is_paused;
  int step;
  float speed;
};

struct fmatrix open_csv(const char *filename) {
  struct fmatrix values;
  fpos_t file_begin;
  FILE *csv_file;
  char endchar = 'a';
  float input;
  int i = 0;
  int prev_j = 0;
  int j = 1;

  csv_file = fopen(filename, "r");
  fgetpos(csv_file, &file_begin);

  while (endchar != EOF) {

    if (fscanf(csv_file, "%f%c", &input, &endchar) != 2) break;
    /* printf("%f at coordinates (%d, 0)\n", input, i); */
    j = 1;

    while (endchar != '\n' && endchar != EOF) {
      fscanf(csv_file, "%f%c", &input, &endchar);
      /* printf("%f at coordinates (%d, %d)\n", input, i, j); */
      j++;
    }

    prev_j = prev_j == 0 ? j : prev_j;

    if (j != prev_j) {
      fprintf(stderr, "Irregular file, line %d has %d values and line %d has %d\n", i, prev_j, i + 1, j);
      values.matrix = malloc(sizeof(float));
      values.h = 0;
      values.w = 0;
      return values;
    }

    i++;
  }

  /* printf("i: %d, j: %d\n", i, j); */

  /* printf("allocating matrix\n"); */

  values.matrix = malloc(sizeof(float) * i * j);
  values.h = i;
  values.w = j;

  /* printf("matrix successfully allocated, starting to fill\n"); */

  fsetpos(csv_file, &file_begin);

  for (i=0; i < values.h * values.w; i++) {
      fscanf(csv_file, "%f%c", &values.matrix[i], &endchar);
  }

  /* printf("Matrix filled entirely\n"); */

  fclose(csv_file);
  return values;
}

int main(int argc, char **argv) {

  // timing
  clock_t prev_t;

  // X11 Variables
  Display *disp;
  GC gc;
  XGCValues gc_attr;
  Window win, root_win;
  XWindowAttributes win_attr_get;
  XSetWindowAttributes win_attr_set;
  XEvent event;
  unsigned long white, black;

  // Position values
  struct fmatrix values;
  float max_dist;
  float highest_pos = 0;
  /* XPoint *object_positions; */
  XArc *object_drawings;
  XSegment *strings;
  float speed;

  // simulation variables
  struct sim_state state;
  int i;

  // Checking args

  if (argc != 3) {
    fprintf(stderr, "usage: %s [file] [speed]\n", argv[0]);
    return 1;
  }

  // Getting values from the provided file

  values = open_csv(argv[1]);

  if (values.h == 0 || values.w == 0) {
    fprintf(stderr, "Unable to read file\n");
    return 1;
  }

  speed = atof(argv[2]);

  // Fixing positive values

  for (i=0; i < values.w * values.h; i++) {
    if (values.matrix[i] > highest_pos) {
      highest_pos = values.matrix[i];
    }
  }

  for (i=0; i < values.w * values.h; i++) {
    values.matrix[i] -= highest_pos;
  }

  // Getting the maximum distance for normalisation

  /* printf("finding maximum distance\n"); */

  for (i=0; i < values.w * values.h; i++) {
    /* printf("%f", values.matrix[i]); */
    if (values.matrix[i] < max_dist) {
      max_dist = values.matrix[i];
    }
  }

  printf("max dist: %f\n", max_dist);

  for (i=0; i < values.w * values.h; i++) {
    values.matrix[i] = values.matrix[i] / max_dist;
    /* printf("%f\n", values.matrix[i]); */
  }

  // Initializing X11 variables (Display/Window/GC)

  /* printf("opening display\n"); */

  disp = XOpenDisplay("");

  if (disp == NULL) {
    fprintf(stderr, "Error opening display\n");
    return 1;
  }

  white = XWhitePixel(disp, DefaultScreen(disp));
  black = XBlackPixel(disp, DefaultScreen(disp));

  win_attr_set.background_pixel = black;
  win_attr_set.border_pixel = white;

  gc_attr.function = GXcopy;
  gc_attr.background = black;
  gc_attr.foreground = white;

  win = XCreateWindow(disp, RootWindow(disp, DefaultScreen(disp)), 0, 0, 500, 500, 0, CopyFromParent, InputOutput, CopyFromParent, CWBackPixel | CWBorderPixel, &win_attr_set);

  gc = XCreateGC(disp, win, GCFunction | GCForeground | GCBackground, &gc_attr);

  XMapWindow(disp, win);
  XSelectInput(disp, win, StructureNotifyMask);
  XNextEvent(disp, &event);
  while (event.type != MapNotify) {
    XNextEvent(disp, &event);
  }

  // initializing sim

  /* printf("allocating object positions\n"); */

  /* object_positions = malloc(sizeof(XPoint) * values.h); */
  object_drawings = malloc(sizeof(XArc) * values.h);
  strings = malloc(sizeof(XSegment) * values.h);

  state.is_running = 1;
  state.is_paused = 0;
  state.step = 0;

  prev_t = clock();

  XSelectInput(disp, win, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | ExposureMask);

  // main loop

  event.type = Expose;
  XSendEvent(disp, win, 0, ExposureMask, &event);

  while (state.is_running == 1) {
    XGetWindowAttributes(disp, win, &win_attr_get);
    XNextEvent(disp, &event);

    if (event.type == KeyPress) {
      printf("%d\n", event.xkey.keycode);
      switch(event.xkey.keycode) {
        case 9:
          state.is_paused = state.is_paused == 1 ? 0 : 1;
          break;
        case 24:
          state.is_running = 0;
          break;
      }
    }

    if (event.type == Expose) {
      /* printf("%f\n", (float)(clock() - prev_t)/CLOCKS_PER_SEC); */
      if ((float)(clock() - prev_t)/CLOCKS_PER_SEC > speed && state.is_paused != 1) {

        XClearWindow(disp, win);

        for (i=0; i < values.h; i++) {
          object_drawings[i].y = (int) (values.matrix[state.step + i * values.w] * 0.8 * (float)win_attr_get.height) - OBJECT_DIAMETER / 2;
          object_drawings[i].x = (win_attr_get.width - OBJECT_DIAMETER) / 2;
          object_drawings[i].angle1 = 0;
          object_drawings[i].angle2 = 360 * 64;
          object_drawings[i].height = OBJECT_DIAMETER;
          object_drawings[i].width = object_drawings[i].height;
          strings[i].y1 = i == 0 ? 0 : object_drawings[i-1].y + object_drawings[i - 1].height / 2;
          strings[i].y2 = object_drawings[i].y;
          strings[i].x1 = win_attr_get.width / 2;
          strings[i].x2 = win_attr_get.width / 2;
        }


        prev_t = clock();
        state.step += 1;

        if (state.step >= values.w) {
          state.is_running = 0;
          printf("Reached end of simulation.");
        }
      }

      XSendEvent(disp, win, 0, ExposureMask, &event);
      XFillArcs(disp, win, gc, object_drawings, values.h);
      XDrawSegments(disp, win, gc, strings, values.h);
    }



    XFlushGC(disp, gc);
  }

  // Closing everything properly

  XUnmapWindow(disp, win);
  XDestroyWindow(disp, win);
  XCloseDisplay(disp);

  return 0;
}
