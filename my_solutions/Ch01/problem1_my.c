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
            "!--Error 12: Cannot parse the barometric pressure value in the line:\n%s",
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
    fprintf(stderr, "!--Error 10: Cannot parse the date value in the line:\n%s\n", str_date);
    return FALSE;
  }

  // Parse time
  if ( sscanf(str_time, "%2d:%2d:%2d", 
              &tm_res->tm_hour, &tm_res->tm_min, &tm_res->tm_sec) != 3 ) {
    fprintf(stderr, "!--Error 11: Cannot parse the time value in the line:\n%s\n", str_time);
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

// Data type for storing the barometric pressure and the corresponding time (in time_t format)
typedef struct
{
  time_t time;
  double press;
} BPT_point;

// Stores the array of BPT points 
typedef struct
{
  long size;
  long capacity;
  BPT_point *arr_bpt;
} BPT_array;

// Functions to work with BPT_array.
int init_BPT(BPT_array *p_arrbpt, int init_capacity)
{
  p_arrbpt->arr_bpt = (BPT_point*)malloc(init_capacity * sizeof(BPT_point));
  if (!p_arrbpt->arr_bpt) {
    fprintf(stderr, "!--Error 20: Cannot allocate the memory to store the BPT_point array\n");
    return FALSE;
  }
  p_arrbpt->capacity = init_capacity;
  p_arrbpt->size = 0;
  return TRUE;
}

int add_BPT_point(BPT_array *p_arrbpt, BPT_point point)
{
  // Increase the array capacity if needed
  if (p_arrbpt->size == p_arrbpt->capacity) {
    long new_capacity = p_arrbpt->capacity * 2; // double the capacity
    BPT_point *new_ptn_arr = (BPT_point*)realloc(p_arrbpt->arr_bpt, new_capacity * sizeof(BPT_point));
    if (!new_ptn_arr) {
      fprintf(stderr, "!--Error 21: Cannot reallocate (increase) the memory to add a new BPT_point instance\n");
      return FALSE;
    }
    p_arrbpt->arr_bpt = new_ptn_arr;
    p_arrbpt->capacity = new_capacity;
    return TRUE;
  }

  // Add a new point
  p_arrbpt->arr_bpt[p_arrbpt->size] = point;
  p_arrbpt->size++;
  return TRUE;
}

void free_BPT(BPT_array *p_arrbpt)
{
  free(p_arrbpt->arr_bpt);
  p_arrbpt->arr_bpt = NULL;
  p_arrbpt->size = p_arrbpt->capacity = 0;
}

void print_BPT(const BPT_array *p_arrbpt)
{
  int i;
  for (i = 0; i < p_arrbpt->size; ++i)
    printf("%ld: %ld %.2f\n", i, p_arrbpt->arr_bpt[i].time, p_arrbpt->arr_bpt[i].press);
}

// Fetch the data for analysis
int fetch_data(FILE *pf, const struct tm *tm_begin, const struct tm *tm_end, BPT_array *p_arrbpt)
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
      break;

    // Convert the current tm value to time_t
    if ((time_curr = mktime(&tm_curr)) == -1) {
      print_tm("!--Error 10: Cannot convert datetime values from tm struct to time_t:\n", 
               &tm_curr, stderr);
      fprintf(stderr, "The wrong datetime value is in the following line read:\n%s",
              buff_line);
      break;
    }

    // Fetch the data for analysis and save it to the BPT_array struct
    if ( time_curr >= time_begin && time_curr <= time_end ) {
      // Parse the barometric pressure value
      if ( parse_pressure(buff_line, &press_curr) == FALSE )
        break;

      // Save the current point press=f(time) for futher analysis
      // print_tm("Datetime to save: ", &tm_curr, stdout);
      // printf("Pressure to save: %f\n", press_curr);
      // printf("---------------\n");
      BPT_point ptn_curr = {time_curr, press_curr};
      if ( add_BPT_point(p_arrbpt, ptn_curr) == FALSE ) {
        print_tm("!--Error 11: Cannot add a new point pressure=f(time) for time: ",
	         &tm_curr, stderr);
        break;
      }
      data_fetched = TRUE;
      continue;
    }

    // Data is saved, this is a SUCCESSFUL EXIT from the function
    if (data_fetched == TRUE) return TRUE;
  }
  return FALSE;
}

// Calc the Slope coefficient of the barometric pressure.
double coeff_slope_press(const BPT_array *bpt)
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
  if (argc != 6) {
    fprintf(stderr, 
            "Usage: %s weather_filename begin_date begin_time end_date end_time\n"
            "where the following formats should be used:\n"
            "  date: yyyy_mm_dd\n"
            "  time: hh:mm:ss\n", argv[0]);
    return 1;
  }

  struct tm tm_begin = {0}, tm_end = {0};
  
  // Create and init the BPT_array struct
  BPT_array arr_bpt;
  if ( init_BPT(&arr_bpt, 10) == FALSE ) {
    fprintf(stderr, "!--Error 2: Cannot init the BPT array struct\n");
    return 2;
  }

  // Open the dat-file
  FILE *pfile = fopen(argv[1], "r");
  if (!pfile) {
    fprintf(stderr, "!--Error 3: Cannot open the file '%s'\n", argv[1]);
    return 3;
  }

  // Parse the passed datetime values of the begin and end of the data range.
  if ( parse_datetime(argv[2], argv[3], &tm_begin) == FALSE || 
       parse_datetime(argv[4], argv[5], &tm_end) == FALSE ) return 3;
  
  // Retrieve the data for analysis
  if ( fetch_data(pfile, &tm_begin, &tm_end, &arr_bpt) == FALSE ) {
    fprintf(stderr, "!--Error 4: Cannot retrieve the data for analysis in the following range:\n");
    print_tm("From: ", &tm_begin, stderr);
    print_tm("To:   ", &tm_end, stderr);
    return 4;
  }
  fclose(pfile);

  // Calc the Slope coefficient
  printf("%ld data points were retrieved for analysis:\n", arr_bpt.size);
  print_BPT(&arr_bpt);
  double k_slope = coeff_slope_press(&arr_bpt);
  printf("Slope coefficient: %d\n", k_slope);

  free_BPT(&arr_bpt);

  return 0;
}

