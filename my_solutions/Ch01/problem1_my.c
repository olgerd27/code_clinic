#include <stdio.h>
#include <time.h>

// TODO: change the correct data types instead of 'date' and 'time' in the function signature.
//
// Returns the slope coefficient of the barometric pressure.
//double coeff_slope_press(FILE *pf, const struct tm *tm_begin, const struct tm *tm_end)
double coeff_slope_press(FILE *pf, 
                         const char *str_date_beg, const char *str_time_beg,
                         const char *str_date_end, const char *str_time_end)
{
  const int LINE_MAX = 100; // the max line length in the dat files 
  char buff[LINE_MAX]; // buffer for storage of the read line
  double k_slope; // the slope coefficient

  const struct tm tm_begin, tm_end;
  
  //const time_t time_begin = mktime((struct tm *)tm_begin);
  //const time_t time_end = mktime((struct tm *)tm_end);

  int i = 0;
  while ( fgets(buff, LINE_MAX, pf) != NULL ) {
    printf("%s", buff);
    if (i++ >5) break;
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
  if (argc != 6) {
    fprintf(stderr, "Usage: %s weather_filename begin_date begin_time end_date end_time\n"
                    "where the following formats should be used:\n"
		    "  date: yyyy_mm_dd\n"
		    "  time: hh:mm:ss\n", argv[0]);
    return 1;
  }
  FILE *pfile = fopen(argv[1], "r");
  if (!pfile) {
    fprintf(stderr, "!---Error: cannot open the file '%s'\n", argv[1]);
    return 1;
  }
  double k_press = coeff_slope_press(pfile, argv[1], argv[2], argv[3], argv[4]);
  fclose(pfile);
  return 0;
}
