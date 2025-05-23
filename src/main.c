#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "raylib.h"

#define NOB_IMPLEMENTATION
#include "../nob.h"

#define STB_DS_IMPLEMENTATION
#include "../third-party/stb_ds.h"

#define GRID_ROW 7
#define GRID_COL 6
#define GRID_PADDING 10

#define TITLE_FONT_SIZE 50 
#define CAL_TEXT_FONT_SIZE 16 

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
  bool vip;
  Nob_String_View name;
  Color bkgColor;
  Color fgColor;
} Day;

typedef struct {
  Day *items;
  size_t capacity;
  size_t count;
} Days;

typedef struct {
  Nob_String_View name;
  struct tm timeinfo;
  Color bkgColor;
  Color fgColor;
} VIPDate;

typedef struct {
  VIPDate *items;
  size_t capacity;
  size_t count;
} Dates;

typedef struct {
  char *key;
  Color value;
} ColorMapItem;

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

bool IsVIPDay(Dates *vips, Day *day) {
  for (size_t i = 0; i < vips->count; ++i) {
    struct tm vtm = vips->items[i].timeinfo;
    if (day->timeinfo.tm_year == vtm.tm_year &&
        day->timeinfo.tm_mon == vtm.tm_mon &&
        day->timeinfo.tm_mday == vtm.tm_mday) {
      day->vip = true;
      day->name = vips->items[i].name;
      day->bkgColor = vips->items[i].bkgColor;
      day->fgColor = vips->items[i].fgColor;
      return true;
    }
  }
  return false;
}

void CreateDays(Days *days, size_t year, size_t month, Dates *vips) {
  size_t daysCount = (size_t)GetDaysInMonth(year, month);
  for (size_t i = 1; i <= daysCount; ++i) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = year-1900;
    timeinfo.tm_mon = month;
    timeinfo.tm_mday = i;
    time_t t = mktime(&timeinfo);
    NOB_UNUSED(t);
    Day d = { .timeinfo = timeinfo, .state = DS_NONE, .today = IsToday(&timeinfo), .vip = false };
    IsVIPDay(vips, &d); 
    nob_da_append(days, d); 
  }
}

bool DrawButton(Rectangle boundary, const char *text, Font font) {
  Color bkgColor = DARKGRAY;
  bool clicked = false;

  if (CheckCollisionPointRec(GetMousePosition(), boundary)) {
    bkgColor = GRAY;
    clicked = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
  }
  
  DrawRectangleRec(boundary, bkgColor);
  Vector2 textDims = MeasureTextEx(font, text, NP_BUTTON_FONT_SIZE, 1);
  Vector2 pos = { .x = (boundary.x+NP_BUTTON_WIDTH/2) - textDims.x/2, .y = (boundary.y+NP_BUTTON_HEIGHT/2)-NP_BUTTON_FONT_SIZE/2 };
  DrawTextEx(font, text, pos, NP_BUTTON_FONT_SIZE, 1, RAYWHITE);

  return clicked;
}

void GetNextMonth(int direction, int *month, size_t *year, Days *days, Dates *vips) {
  *month += direction; 
  if (*month > 11) {
    *month = 0;
    *year += 1;
  }
  if (*month < 0) {
    *month = 11;
    *year -= 1;
  }
  days->count = 0;
  CreateDays(days, *year, *month, vips);
}

void FillColorMap(ColorMapItem **map) {
  shput(*map, "LIGHTGRAY", LIGHTGRAY);  
  shput(*map, "GRAY", GRAY);
  shput(*map, "DARKGRAY", DARKGRAY);
  shput(*map, "YELLOW", YELLOW); 
  shput(*map, "GOLD", GOLD);
  shput(*map, "ORANGE", ORANGE);
  shput(*map, "PINK", PINK);
  shput(*map, "RED", RED);
  shput(*map, "MAROON", MAROON);
  shput(*map, "GREEN", GREEN);
  shput(*map, "LIME", LIME);
  shput(*map, "DARKGREEN", DARKGREEN);
  shput(*map, "SKYBLUE", SKYBLUE);
  shput(*map, "BLUE", BLUE);
  shput(*map, "DARKBLUE", DARKBLUE);
  shput(*map, "PURPLE", PURPLE);
  shput(*map, "VIOLET", VIOLET);
  shput(*map, "DARKPURPLE", DARKPURPLE);
  shput(*map, "BEIGE", BEIGE);
  shput(*map, "BROWN", BROWN);
  shput(*map, "DARKBROWN", DARKBROWN);
  shput(*map, "WHITE", WHITE);
  shput(*map, "BLACK", BLACK);
  shput(*map, "BLANK", BLANK);
  shput(*map, "MAGENTA", MAGENTA);
  shput(*map, "RAYWHITE", RAYWHITE);
}

bool ParseYaml(Dates *dates) {
  const char *path = "./dates.yaml";

  Nob_String_Builder sb = {0};
  if (!nob_read_entire_file(path, &sb)) return false;

  Nob_String_View sv = nob_sb_to_sv(sb); 
  Nob_String_View line = nob_sv_chop_by_delim(&sv, '\n');

  ColorMapItem *colorMap = {0};
  FillColorMap(&colorMap);

  VIPDate d = {0};
  while(line.count) {
    if (line.data[0] != ' ') {
      d.name.count = 0;
      d.name = nob_sv_chop_by_delim(&line, ':');
    } else {
      line = nob_sv_trim(line);
      Nob_String_View thing = nob_sv_chop_by_delim(&line, ':');
      if (nob_sv_eq(thing, nob_sv_from_cstr("date"))) {
        line = nob_sv_trim(line);
        Nob_String_View year = nob_sv_chop_left(&line, 4);
        Nob_String_View month = nob_sv_chop_left(&line, 2);
        Nob_String_View day = nob_sv_chop_left(&line, 2);
        int yeari = atoi(nob_temp_sv_to_cstr(year));
        int monthi = atoi(nob_temp_sv_to_cstr(month));
        int dayi = atoi(nob_temp_sv_to_cstr(day));
      
        struct tm date = {0};
        date.tm_year = yeari - 1900;
        date.tm_mon = monthi -1;
        date.tm_mday = dayi;
        time_t t = mktime(&date);
        NOB_UNUSED(t);
        d.timeinfo = date;

      } else if (nob_sv_eq(thing, nob_sv_from_cstr("bkgColor"))) {
        const char* color = nob_temp_sv_to_cstr(nob_sv_trim(line));
        Color c = shget(colorMap, color);
        nob_log(NOB_INFO, "%d", c.r);
        d.bkgColor = c; 
      } else if (nob_sv_eq(thing, nob_sv_from_cstr("fgColor"))) {
        const char* color = nob_temp_sv_to_cstr(nob_sv_trim(line));
        d.fgColor = shget(colorMap, color);
        nob_da_append(dates, d);
      }
    } 
    line = nob_sv_chop_by_delim(&sv, '\n');
  }
  return true;
}

int main(void) {
  Dates vips = {0};
  if (!ParseYaml(&vips)) return 1;

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(1200, 720, "calendar");

  //#define TITLE_FONT_SIZE 50 
  //#define CAL_TEXT_FONT_SIZE 16 

  Font arvoText = LoadFontEx("./assets/Arvo/Arvo-Regular.ttf", CAL_TEXT_FONT_SIZE, NULL, 0);
  Font arvoTitle = LoadFontEx("./assets/Arvo/Arvo-Regular.ttf", TITLE_FONT_SIZE, NULL, 0);
  SetTextureFilter(arvoText.texture, TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(arvoTitle.texture, TEXTURE_FILTER_TRILINEAR);


  Font TITLE_FONT = arvoTitle;
  Font TEXT_FONT = arvoText;

  int MONTH_IDX = 0;
  size_t YEAR = 2025;
  Day *selectedDay = NULL;
  Days days = {0};
  CreateDays(&days, YEAR, MONTH_IDX, &vips);

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
    if (DrawButton(prev_btn_rect, prevText, TEXT_FONT)) GetNextMonth(-1, &MONTH_IDX, &YEAR, &days, &vips);
    Rectangle next_btn_rect = { .x = (prev_btn_rect.x + prev_btn_rect.width)+GRID_PADDING, .y = GRID_PADDING, .width = NP_BUTTON_WIDTH, .height = NP_BUTTON_HEIGHT };
    const char* nextText = "NEXT";
    if (DrawButton(next_btn_rect, nextText, TEXT_FONT)) GetNextMonth(1, &MONTH_IDX, &YEAR, &days, &vips);

    char yearText[5];
    sprintf(yearText, "%ld", YEAR);

    char title[strlen(MONTH_NAMES[MONTH_IDX])+strlen(yearText)+2];
    strcpy(title, MONTH_NAMES[MONTH_IDX]);
    strcat(title, " ");
    strcat(title, yearText);
    Vector2 nameDims = MeasureTextEx(TITLE_FONT, title, TITLE_FONT_SIZE, 0);
    Vector2 titlePos = { .x = GetScreenWidth()/2-nameDims.x/2, .y = 0 };
    DrawTextEx(TITLE_FONT, title, titlePos, TITLE_FONT_SIZE, 0, RAYWHITE);  

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
        } else if (d->vip) {
          DrawRectangleRec(r, d->bkgColor);
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
          fs = CAL_TEXT_FONT_SIZE*1.25;
        }
        if (d->vip) {
          c = d->fgColor;
        }
        DrawTextEx(TEXT_FONT, text, CLITERAL(Vector2){x+4, y+4}, fs, 0, c);
        if (d->vip) {
          DrawTextEx(TEXT_FONT, nob_temp_sv_to_cstr(d->name), CLITERAL(Vector2){x+4, y+fs*1.5+4}, fs*1.5, 0, c);
        }
        daysIdx++;
      } 
      
      DrawRectangleLinesEx(r, 1, RAYWHITE);

    }

    EndDrawing();
  }

  UnloadFont(arvoTitle);
  UnloadFont(arvoTitle);

  CloseWindow();

  return 0;
}
