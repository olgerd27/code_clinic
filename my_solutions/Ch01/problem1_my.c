#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Do the parsing of the date & time strings and set the parsed values into the tm struct instance.
int parse_datetime(const char *str_date, const char *str_time, struct tm *tm_res)
{
  int nitems = sscanf(str_date, "%4d_%2d_%2d", &tm_res->tm_year, &tm_res->tm_mon, &tm_res->tm_mday);
  nitems += sscanf(str_time, "%2d:%2d:%2d", &tm_res->tm_hour, &tm_res->tm_min, &tm_res->tm_sec);
  return nitems == 6 ? 0 : 1;
}

// Set the values of the tm struct instance in a way of parsing 
// the passed date & time string-type arguments.
void set_tm(const char *str_date, const char *str_time, struct tm *tm_res)
{
  if ( parse_datetime(str_date, str_time, tm_res) != 0 ) {
    fprintf(stderr, 
            "!--Error 2: cannot parse the date&time value.\ndate: '%s'\ntime: '%s'\n", 
            str_date, str_time);
    exit(2);
  }
}

// Print to STDOUT the values of the tm struct instance.
void print_tm(const struct tm *tm_data, const char *comment)
{
  printf("%s:\n%4d-%02d-%02d, %02d:%02d:%02d\n", 
         comment, 
         tm_data->tm_year, tm_data->tm_mon, tm_data->tm_mday, 
         tm_data->tm_hour, tm_data->tm_min, tm_data->tm_sec);
}

// Calc the Slope coefficient of the barometric pressure.
double coeff_slope_press(FILE *pf, const struct tm *tm_begin, const struct tm *tm_end)
{
  const int LINE_MAX = 150; // the max line length in the dat files 
  char buff_line[LINE_MAX]; // buffer for storage of the read line
  struct tm tm_curr; // the tm value read in the current line
  time_t time_curr; // the time_t value in the current line
  double k_slope; // the slope coefficient

  const time_t time_begin = mktime((struct tm *)tm_begin);
  const time_t time_end = mktime((struct tm *)tm_end);

  // Reading the file to retrieve the data for analysis
  int i = 0;
  (void)fgets(buff_line, LINE_MAX, pf); // skip the header
  while ( fgets(buff_line, LINE_MAX, pf) != NULL ) {
    set_tm(buff_line, strchr(buff_line, ' '), &tm_curr); 
    print_tm(&tm_curr, "Current tm");

    // Convert the current tm value to time_t
    if ((time_curr = mktime(&tm_curr)) == -1) {
      fprintf(stderr, "!---Error 3: wrong datetime value in the dat-file:\n'%s'\n", buff_line);
      exit(3);
    }

    // Compare
    //if (time_curr >= time_begin) 

    if (++i > 5) break;
  }
  return k_slope;
}

/*
 * Algorithm:
 * 1. Get user input of begin & end of the data & time.
 * 2. Open the file.
 * 3. 
 *
 * NOTE: X values: x_curr = time_in_sec_curr - time_in_sec_beg:
 * Use mktime to convert tm value to time_t, and then difftime() to calc a difference.
 * 
 * NOTE: The exact date and time values entered by the user may not exist in the file.
 *       So, inputted values may be either between the values in the file, 
 *       or be the same as in the file. 
 *       That means we cannot just compare the date&time in string format.
 *
 * An example of the program execution:
 * ./a.out Env_Data_Deep_Moor_2012_part.txt 2012_04_01 00:00:00 2012_05_01 23:59:59
 */
int main(int argc, char *argv[])
{
  struct tm tm_begin = {0}, tm_end = {0};
  if (argc != 6) {
    fprintf(stderr, "Usage: %s weather_filename begin_date begin_time end_date end_time\n"
                    "where the following formats should be used:\n"
		    "  date: yyyy_mm_dd\n"
		    "  time: hh:mm:ss\n", argv[0]);
    return 1;
  }

  // Open the dat-file
  FILE *pfile = fopen(argv[1], "r");
  if (!pfile) {
    fprintf(stderr, "!---Error: cannot open the file '%s'\n", argv[1]);
    return 1;
  }

  // Set the the tm struct instances of the begin and end of the data range 
  // (for further analysis) by parsing the passed arguments.
  set_tm(argv[2], argv[3], &tm_begin);
  set_tm(argv[4], argv[5], &tm_end);
  // print_tm(&tm_begin, "The begin of the date & time range");
  // print_tm(&tm_end, "The end of the date & time range");
  
  // Calc the Slope coefficient
  double k_press = coeff_slope_press(pfile, &tm_begin, &tm_end);

  // Close the file
  fclose(pfile);

  return 0;
}
