%module pyccze
%{
#define SWIG_FILE_WITH_INIT
#include "ccze.h"
%}
int ccze_color_keyword_lookup(const char *key);
void ccze_print_date(const char *date);
char *ccze_strbrk(char *str, char delim);
void ccze_newline(void);
void ccze_space(void);
void ccze_wordcolor_process_one(char *word, int slookup);
static char *ccze_str_htmlencode(const char *src);
void ccze_addstr(ccze_color_t col, const char *str);
ccze_color_t ccze_http_action(const char *method);

void ccze_plugin_add(ccze_plugin_t *plugin);
