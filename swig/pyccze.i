%module pyccze
%{
#define SWIG_FILE_WITH_INIT
#include "ccze.h"
%}
int ccze_color_keyword_lookup (const char *key);
void ccze_print_date (const char *date);
char *ccze_strbrk (char *str, char delim);
char *xstrdup (const char *str);
void ccze_newline (void);
void ccze_space (void);
void ccze_wordcolor_process_one (char *word, int slookup);
