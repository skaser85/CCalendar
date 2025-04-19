#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "raylib.h"

#define NOB_IMPLEMENTATION
#include "../nob.h"

#define GRID_ROW 7
#define GRID_COL 6
#define GRID_PADDING 10

#define TITLE_FONT_SIZE 50
#define CAL_TEXT_FONT_SIZE 12

#define NP_BUTTON_WIDTH 100
#define NP_BUTTON_HEIGHT 50
#define NP_BUTTON_FONT_SIZE 30

const char *DAY_NAMES[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const char *MONTH_NAMES[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

typedef enum {
  DS_NONE,
  DS_HOVERED,
  DS_SELECTED
} DayState;

typedef struct {
  struct tm timeinfo;
  DayState state;
  bool today;
} Day;

typedef struct {
  Day *items;
  size_t capacity;
  size_t count;
} Days;

bool IsToday(struct tm *date) {
  time_t t = time(NULL);
  struct tm *current = localtime(&t);

  return (date->tm_year == current->tm_year &&
          date->tm_mon == current->tm_mon &&
          date->tm_mday == current->tm_mday);
}

int GetDaysInMonth(size_t year, size_t month) {
  if (month == 3 || month == 5 || month == 8 || month == 10) {
    return 30;
  } else if (month == 1) {
    return ((year %4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 29 : 28;
  } else {
    return 31;
  }
}

void CreateDays(Days *days, size_t year, size_t month) {
  size_t daysCount = (size_t)GetDaysInMonth(year, month);
  for (size_t i = 1; i <= daysCount; ++i) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = year-1900;
    timeinfo.tm_mon = month;
    timeinfo.tm_mday = i;
    time_t t = mktime(&timeinfo);
    NOB_UNUSED(t);
    Day d = { .timeinfo = timeinfo, .state = DS_NONE, .today = IsToday(&timeinfo) };
    nob_da_append(days, d); 
  }
}

bool DrawButton(Rectangle boundary, const char *text) {
  Color bkgColor = DARKGRAY;
  bool clicked = false;

  if (CheckCollisionPointRec(GetMousePosition(), boundary)) {
    bkgColor = GRAY;
    clicked = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
  }
  
  DrawRectangleRec(boundary, bkgColor);
  int textW = MeasureText(text, NP_BUTTON_FONT_SIZE);
  DrawText(text, (boundary.x+NP_BUTTON_WIDTH/2) - textW/2, (boundary.y+NP_BUTTON_HEIGHT/2)-NP_BUTTON_FONT_SIZE/2, NP_BUTTON_FONT_SIZE, RAYWHITE);

  return clicked;
}

int GetNextMonth(int direction, int month, size_t year, Days *days) {
  month += direction; 
  if (month > 11)
    month = 0;
  if (month < 0)
    month = 11;
  days->count = 0;
  CreateDays(days, year, month);
  return month;
}

int main(void) {
  InitWindow(1680, 1050, "calendar");

  int MONTH_IDX = 0;
  size_t YEAR = 2025;
  Day *selectedDay = NULL;
  Days days = {0};
  CreateDays(&days, YEAR, MONTH_IDX);

  size_t start_x = GRID_PADDING;
  size_t end_x = GetScreenWidth()-GRID_PADDING;
  size_t gridW = (end_x-start_x)/GRID_ROW;

  size_t start_y = TITLE_FONT_SIZE+GRID_PADDING*2;
  size_t end_y = GetScreenHeight()-GRID_PADDING;
  size_t gridH = (end_y-start_y)/GRID_COL;

  
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(GetColor(0x181818));
  
    Rectangle prev_btn_rect = { .x = GRID_PADDING, .y = GRID_PADDING, .width = NP_BUTTON_WIDTH, .height = NP_BUTTON_HEIGHT };
    const char* prevText = "PREV";
    if (DrawButton(prev_btn_rect, prevText)) MONTH_IDX = GetNextMonth(-1, MONTH_IDX, YEAR, &days);
    Rectangle next_btn_rect = { .x = (prev_btn_rect.x + prev_btn_rect.width)+GRID_PADDING, .y = GRID_PADDING, .width = NP_BUTTON_WIDTH, .height = NP_BUTTON_HEIGHT };
    const char* nextText = "NEXT";
    if (DrawButton(next_btn_rect, nextText)) MONTH_IDX = GetNextMonth(1, MONTH_IDX, YEAR, &days);

    char yearText[5];
    sprintf(yearText, "%ld", YEAR);

    char title[strlen(MONTH_NAMES[MONTH_IDX])+strlen(yearText)+2];
    strcpy(title, MONTH_NAMES[MONTH_IDX]);
    strcat(title, " ");
    strcat(title, yearText);
    int nameW = MeasureText(title, TITLE_FONT_SIZE);
    DrawText(title, GetScreenWidth()/2-nameW/2, 0, TITLE_FONT_SIZE, RAYWHITE);  

    size_t row = 0;
    size_t daysIdx = 0;
    bool found = false;
    for (size_t i = 0; i < GRID_ROW*GRID_COL; ++i) {
      
      size_t x = start_x+((i%7)*gridW);
      if (i > 0 && i%7==0) row++;
      size_t y = start_y+(row*gridH);
      Rectangle r = { .x = x, .y = y, .width = gridW, .height = gridH };

      Day *d = &days.items[daysIdx];
      if (!found && d->timeinfo.tm_wday == (int)i) found = true;
            
      if (found && daysIdx < days.count) {
        if (d->state != DS_SELECTED) {
          if(CheckCollisionPointRec(GetMousePosition(), r)) {
            if (d->state == DS_NONE) d->state = DS_HOVERED;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
              d->state = DS_SELECTED;
              if (selectedDay) {
                selectedDay->state = DS_NONE;
              } 
              selectedDay = d;
            }
          } else {
            d->state = DS_NONE;
          }
        }

        if (d->state == DS_HOVERED) {
          DrawRectangle(x, y, gridW, gridH, DARKGRAY);
        } else if (d->state == DS_SELECTED) {
          DrawRectangleRec(r, DARKPURPLE);
        }


        char numText[2];
        sprintf(numText, "%d", d->timeinfo.tm_mday);
        char text[20];
        strcpy(text, numText);
        strcat(text, " - ");
        strcat(text, DAY_NAMES[d->timeinfo.tm_wday]);
        Color c = RAYWHITE;
        int fs = CAL_TEXT_FONT_SIZE;
        if (d->today) {
          c = LIME;
          fs = CAL_TEXT_FONT_SIZE*2;
        }
        DrawText(text, x+2, y+2, fs, c);
        daysIdx++;
      } 
      
      DrawRectangleLinesEx(r, 1, RAYWHITE);

    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
