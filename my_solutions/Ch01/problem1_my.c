#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

// Parse the barometric pressure value.
int parse_pressure(const char *str_press, double *press_res)
{
  if ( sscanf(strchr(str_press, '\t') + 1, "%lf ", press_res) == 0 ) {
    fprintf(stderr, 
            "!--Error 12: cannot parse the barometric pressure value in the line:\n%s",
            str_press);
    return FALSE;
  }
  return TRUE;
}

// Parse the date & time strings and set the parsed values into the tm struct instance.
int parse_datetime(const char *str_date, const char *str_time, struct tm *tm_res)
{
  // Parse date
  if ( sscanf(str_date, "%4d_%2d_%2d", 
              &tm_res->tm_year, &tm_res->tm_mon, &tm_res->tm_mday) != 3 ) {
    fprintf(stderr, "!--Error 10: cannot parse the date value in the line:\n%s\n", str_date);
    return FALSE;
  }

  // Parse time
  if ( sscanf(str_time, "%2d:%2d:%2d", 
              &tm_res->tm_hour, &tm_res->tm_min, &tm_res->tm_sec) != 3 ) {
    fprintf(stderr, "!--Error 11: cannot parse the time value in the line:\n%s\n", str_time);
    return FALSE;
  }

  return TRUE;
}

// Print the values of the tm struct instance with the comment to the fptr output stream.
void print_tm(const char *comment, const struct tm *tm_data, FILE *fptr)
{
  fprintf(fptr, "%s%4d-%02d-%02d, %02d:%02d:%02d\n", 
          comment, 
          tm_data->tm_year, tm_data->tm_mon, tm_data->tm_mday, 
          tm_data->tm_hour, tm_data->tm_min, tm_data->tm_sec);
}

// Data type for storing barometric pressure and corresponding time (in time_t format) data
typedef struct {
  long len;
  time_t *arr_time;
  double *arr_press;
} data_BPT;

void print_BPT(const data_BPT *data)
{
  int i;
  for (i = 0; i < data->len; ++i)
    printf("%ld: %lld %f\n", i, data->arr_time[i], data->arr_press[i]);
}

// Fetch the data for analysis
int fetch_data(FILE *pf, const struct tm *tm_begin, const struct tm *tm_end, data_BPT *bpt_data)
{
  const int LINE_MAX = 150; // the max line length in the dat files 
  char buff_line[LINE_MAX]; // buffer for storage of the read line
  struct tm tm_curr; // the tm value read in the current line
  time_t time_curr; // the time_t value in the current line
  double press_curr; // the barometric pressure value in the current line
  short data_fetched = FALSE; // flad that data has fetched for analysis

  const time_t time_begin = mktime((struct tm *)tm_begin);
  const time_t time_end = mktime((struct tm *)tm_end);

  // Read the dat-file line by line
  long i = 0;
  (void)fgets(buff_line, LINE_MAX, pf); // skip the header
  while ( fgets(buff_line, LINE_MAX, pf) != NULL ) {
    // Parse the datetime value
    if ( parse_datetime(buff_line, strchr(buff_line, ' '), &tm_curr) == FALSE )
      return FALSE;
    // print_tm("Current tm: ", &tm_curr, stdout);

    // Convert the current tm value to time_t
    if ((time_curr = mktime(&tm_curr)) == -1) {
      print_tm("!--Error 4: cannot convert datetime values from tm struct to time_t:\n", 
               &tm_curr, stderr);
      fprintf(stderr, "The wrong datetime value is in the following line read:\n%s",
              buff_line);
      return FALSE;
    }

    // Fetch the data for analysis and save it to the data_BPT struct
    if ( time_curr >= time_begin && time_curr <= time_end ) {
      // Parse the barometric pressure value
      if ( parse_pressure(buff_line, &press_curr) == FALSE )
        return FALSE;
/*    if ( sscanf(strchr(buff_line, '\t') + 1, "%lf ", &press_curr) == 0 ) {
        fprintf(stderr, 
	        "!--Error 5: cannot parse the barometric pressure value in the line:\n%s",
		buff_line);
        return FALSE;
      }
*/
      print_tm("Datetime to save: ", &tm_curr, stdout);
      printf("Pressure to save: %f\n", press_curr);
      printf("---------------\n");
      //save_datapoint(time_curr, press_curr);
      data_fetched = TRUE;
      continue;
    }

    // Data is saved, let's get out of here
    if (data_fetched == TRUE) return TRUE;
    if (++i > 50) break; // TODO: delete
  }
  return FALSE;
}

// Calc the Slope coefficient of the barometric pressure.
double coeff_slope_press(const data_BPT *bpt)
{
  double k_slope; // the slope coefficient
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
  data_BPT bpt_data = {0, 0, 0}; // the barometric pressure and the corresponding time data 
  if (argc != 6) {
    fprintf(stderr, 
            "Usage: %s weather_filename begin_date begin_time end_date end_time\n"
            "where the following formats should be used:\n"
            "  date: yyyy_mm_dd\n"
            "  time: hh:mm:ss\n", argv[0]);
    return 1;
  }

  // Open the dat-file
  FILE *pfile = fopen(argv[1], "r");
  if (!pfile) {
    fprintf(stderr, "!--Error 2: cannot open the file '%s'\n", argv[1]);
    return 2;
  }

  // Parse the passed datetime values of the begin and end of the data range.
  if ( parse_datetime(argv[2], argv[3], &tm_begin) == FALSE || 
       parse_datetime(argv[4], argv[5], &tm_end) == FALSE ) return 3;
  
  // Retrieve the data for analysis
  if ( fetch_data(pfile, &tm_begin, &tm_end, &bpt_data) == FALSE ) {
    fprintf(stderr, "!--Error 4: Cannot retrieve the data for analysis\n");
    print_tm("Data range from: ", &tm_begin, stderr);
    print_tm("Data range to: ", &tm_begin, stderr);
    return 4;
  }
  fclose(pfile);

  // Calc the Slope coefficient
  printf("%ld data points were retrieved for analysis:\n", bpt_data.len);
  //print_BPT(&bpt_data);
  double k_slope = coeff_slope_press(&bpt_data);
  printf("Slope coefficient: %d\n", k_slope);

  return 0;
}

