#include <ccze.h>
#include <stdlib.h>
#include <string.h>

static void ccze_candlepin_setup (void);
static void ccze_candlepin_shutdown (void);
static int ccze_candlepin_handle (const char *str, size_t length, char **rest);

static pcre *reg_candlepin_access, *reg_candlepin_other, *reg_candlepin_header, *reg_candlepin_http_req, *reg_candlepin_http_resp;
static pcre_extra *hints_candlepin_access, *hints_candlepin_other;
static pcre_extra *hints_candlepin_header, *hints_candlepin_http_req, *hints_candlepin_http_resp;

static ccze_color_t _ccze_candlepin_level (const char *level) {
  if (strstr (level, "DEBUG") || strstr (level, "INFO"))
    return CCZE_COLOR_DEFAULT;
  if (strstr (level, "WARN"))
    return CCZE_COLOR_WARNING;
  if (strstr (level, "ERROR"))
    return CCZE_COLOR_ERROR;
  return CCZE_COLOR_UNKNOWN;
}

static ccze_color_t ccze_http_code (const char *code) {
  if (strstr(code, "2")) {
      return CCZE_COLOR_STATIC_GREEN;
  } else if (strstr(code, "4")) {
      return CCZE_COLOR_STATIC_BLUE;
  } else if (strstr(code, "5")) {
      return CCZE_COLOR_ERROR;
  } else if (strstr(code, "3")) {
      return CCZE_COLOR_STATIC_YELLOW;
  } else {
      return CCZE_COLOR_DEFAULT;
  }
}

static char * ccze_candlepin_header_process (const char *srt, int *offsets, int match) {
  char *header, *value;

  pcre_get_substring(srt, offsets, match, 1, (const char**)&header);
  pcre_get_substring(srt, offsets, match, 2, (const char**)&value);

  ccze_addstr(CCZE_COLOR_STATIC_WHITE, header);
  ccze_addstr(CCZE_COLOR_DEFAULT, ":");
  ccze_addstr(CCZE_COLOR_STATIC_GREEN, value);
  ccze_newline();

  free(header);
  free(value);
  return NULL;
}

static char * ccze_candlepin_access_log_process (const char *str, int *offsets, int match) {
  char *date, *time, *mdc, *log_level, *class, *message;
  char *req, *verb, *uri;
  char *resp, *code, *ms;
  ccze_color_t lcol;
  int submatch, suboffsets[99];

  pcre_get_substring(str, offsets, match, 1, (const char**)&date);
  pcre_get_substring(str, offsets, match, 2, (const char**)&time);
  pcre_get_substring(str, offsets, match, 3, (const char**)&mdc);
  pcre_get_substring(str, offsets, match, 4, (const char**)&log_level);
  pcre_get_substring(str, offsets, match, 5, (const char**)&class);
  pcre_get_substring(str, offsets, match, 6, (const char**)&message);

  ccze_addstr(CCZE_COLOR_STATIC_GREEN, date);
  ccze_space();
  ccze_addstr(CCZE_COLOR_DATE, time);
  ccze_space();

  ccze_addstr(CCZE_COLOR_STATIC_BLUE, mdc);
  ccze_space();

  lcol = _ccze_candlepin_level(log_level);
  ccze_addstr(lcol, log_level);
  ccze_space();

  ccze_addstr(CCZE_COLOR_STATIC_MAGENTA, class);
  ccze_space();

  ccze_addstr(CCZE_COLOR_DEFAULT, "-");
  ccze_space();

  if ((submatch = pcre_exec(reg_candlepin_http_req, hints_candlepin_http_req,
              message, strlen(message), 0, 0, suboffsets, 99)) >= 0) {
      pcre_get_substring(message, suboffsets, submatch, 1, (const char**)&req);
      pcre_get_substring(message, suboffsets, submatch, 2, (const char**)&verb);
      pcre_get_substring(message, suboffsets, submatch, 3, (const char**)&uri);

      ccze_addstr(CCZE_COLOR_DEFAULT, req);
      ccze_space();
      ccze_addstr(ccze_http_action(verb), verb);
      ccze_space();
      ccze_addstr(CCZE_COLOR_STATIC_WHITE, uri);
      free(req);
      free(verb);
      free(uri);
  } else if ((submatch = pcre_exec(reg_candlepin_http_resp, hints_candlepin_http_resp,
              message, strlen(message), 0, 0, suboffsets, 99)) >= 0) {
      pcre_get_substring(message, suboffsets, submatch, 1, (const char**)&resp);
      pcre_get_substring(message, suboffsets, submatch, 2, (const char**)&code);
      pcre_get_substring(message, suboffsets, submatch, 3, (const char**)&ms);

      ccze_addstr(CCZE_COLOR_DEFAULT, resp);
      ccze_space();
      ccze_addstr(ccze_http_code(code), code);
      ccze_addstr(CCZE_COLOR_DEFAULT, ms);
      free(resp);
      free(code);
      free(ms);
  } else {
      ccze_addstr(CCZE_COLOR_DEFAULT, message);
  }

  ccze_newline();

  free(date);
  free(time);
  free(mdc);
  free(log_level);
  free(class);
  free(message);
  return NULL;
}

static char * ccze_candlepin_other_log_process (const char *str, int *offsets, int match) {
  char *msg;
  pcre_get_substring(str, offsets, match, 1, (const char **)&msg);

  ccze_addstr(CCZE_COLOR_DEFAULT, msg);
  ccze_newline();

  free(msg);

  return NULL;
}

static void ccze_candlepin_setup (void) {
  const char *error;
  int errptr;

  reg_candlepin_access = pcre_compile
    ("^(\\d{4}-\\d{2}-\\d{2})"
     "\\s(\\d{2}:\\d{2}:\\d{2},\\d{3})"
     "\\s(\\[.*?\\])"
     "\\s(\\w+)"
     "\\s(.+?)\\s-\\s"
     "(.*)$",
     0, &error, &errptr, NULL);
  hints_candlepin_access = pcre_study(reg_candlepin_access, 0, &error);

  reg_candlepin_http_req = pcre_compile
    ("(Request:) (\\w+) (.*)",
     0, &error, &errptr, NULL);
  hints_candlepin_http_req = pcre_study(reg_candlepin_http_req, 0, &error);

  reg_candlepin_http_resp = pcre_compile
    ("(Response:) (\\d{3} .+)(\\s+\\(.*)",
     0, &error, &errptr, NULL);
  hints_candlepin_http_resp = pcre_study(reg_candlepin_http_resp, 0, &error);

  reg_candlepin_header = pcre_compile
    ("^(.*?):(.*)$",
     0, &error, &errptr, NULL);
  hints_candlepin_header = pcre_study(reg_candlepin_header, 0, &error);

  reg_candlepin_other = pcre_compile
    ("^(.*)$",
     0, &error, &errptr, NULL);
  hints_candlepin_other = pcre_study(reg_candlepin_other, 0, &error);
}

static void ccze_candlepin_shutdown (void) {
  free (reg_candlepin_access);
  free (hints_candlepin_access);
  free (reg_candlepin_other);
  free (hints_candlepin_other);
}

static int ccze_candlepin_handle (const char *str, size_t length, char **rest) {
  int match, offsets[99];

  if ((match = pcre_exec (reg_candlepin_access, hints_candlepin_access,
              str, length, 0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_candlepin_access_log_process(str, offsets, match);
      return 1;
    }
 if ((match = pcre_exec (reg_candlepin_header, hints_candlepin_header,
              str, length, 0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_candlepin_header_process(str, offsets, match);
      return 1;
    }
 // For everything else...
 if ((match = pcre_exec (reg_candlepin_other, hints_candlepin_other,
              str, length, 0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_candlepin_other_log_process(str, offsets, match);
      return 1;
    }
  return 0;
}

CCZE_DEFINE_PLUGIN (candlepin, FULL, "Coloriser for candlepin log.");
