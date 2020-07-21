
#ifndef _SETLOCALE_H_
#define _SETLOCALE_H_

struct lc_time_T {
    const char *mon[12];
    const char *month[12];
    const char *wday[7];
    const char *weekday[7];
    const char *X_fmt;
    const char *x_fmt;
    const char *c_fmt;
    const char *am_pm[2];
    const char *date_fmt;
    const char *alt_month[12];    /* unused */
    const char *md_order;
    const char *ampm_fmt;
    const char *era;
    const char *era_d_fmt;
    const char *era_d_t_fmt;
    const char *era_t_fmt;
    const char *alt_digits;
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
    const char	*codeset;	 /* codeset for mbtowc conversion */
  const wchar_t	*wmon[12];
  const wchar_t	*wmonth[12];
  const wchar_t	*wwday[7];
  const wchar_t	*wweekday[7];
  const wchar_t	*wX_fmt;
  const wchar_t	*wx_fmt;
  const wchar_t	*wc_fmt;
  const wchar_t	*wam_pm[2];
  const wchar_t	*wdate_fmt;
  const wchar_t	*wampm_fmt;
  const wchar_t	*wera;
  const wchar_t	*wera_d_fmt;
  const wchar_t	*wera_d_t_fmt;
  const wchar_t	*wera_t_fmt;
  const wchar_t	*walt_digits;
#endif
};
extern const struct lc_time_T _C_time_locale;

static inline const struct lc_time_T *
__get_current_time_locale(void) {
    return &_C_time_locale;
}

struct __locale_t {};
#endif //_SETLOCALE_H_
