#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "raylib.h"

#define NOB_IMPLEMENTATION
#include "../nob.h"

typedef struct {
  Nob_String_View name;
  struct tm timeinfo;
  Nob_String_View bkgColor;
  Nob_String_View fgColor;
} VIPDate;

typedef struct {
  VIPDate *items;
  size_t capacity;
  size_t count;
} Dates;


