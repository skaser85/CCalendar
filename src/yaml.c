#include "./shared.h"

void ResetVIPDate(VIPDate *d) {
  d->name.count = 0; 
}

int main(void) {
 
  const char *path = "./dates.yaml";

  Nob_String_Builder sb = {0};
  if (!nob_read_entire_file(path, &sb)) return 1;

  Nob_String_View sv = nob_sb_to_sv(sb); 
  Nob_String_View line = nob_sv_chop_by_delim(&sv, '\n');
 
  Dates dates = {0};
  VIPDate d = {0};
  while(line.count) {
    if (line.data[0] != ' ') {
      ResetVIPDate(&d);
      d.name = nob_sv_chop_by_delim(&line, ':');
    } else {
      line = nob_sv_trim(line);
      while(line.count) {
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
          d.bkgColor = line;
        } else if (nob_sv_eq(thing, nob_sv_from_cstr("fgColor"))) {
          d.fgColor = line;
          nob_da_append(&dates, d);
        }
      }
    } 
    line = nob_sv_chop_by_delim(&sv, '\n');
  }

  //nob_log(NOB_INFO, "%ld", dates.count);
  for (size_t i = 0; i < dates.count; ++i) {
    VIPDate d = dates.items[i];
    nob_log(NOB_INFO, SV_Fmt, SV_Arg(d.name));
    nob_log(NOB_INFO, "%d-%d-%d", d.timeinfo.tm_year, d.timeinfo.tm_mon, d.timeinfo.tm_mday);
    nob_log(NOB_INFO, SV_Fmt, SV_Arg(d.bkgColor));
    nob_log(NOB_INFO, SV_Fmt, SV_Arg(d.fgColor));
  }
  
  return 0;
}
