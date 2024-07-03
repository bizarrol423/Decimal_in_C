#ifndef SRC_s_DECIMAL_H_
#define SRC_s_DECIMAL_H_

#include <limits.h>
#include <math.h>
#include <stdio.h>

#define POSITIVE_SIGN 0
#define NEGATIVE_SIGN 1

typedef struct {
  unsigned bits[4];
} s_decimal;

#define DEC_NULL   \
  (s_decimal) {  \
    { 0, 0, 0, 0 } \
  }

#define DEC_1      \
  (s_decimal) {  \
    { 1, 0, 0, 0 } \
  }

#define DEC_4      \
  (s_decimal) {  \
    { 4, 0, 0, 0 } \
  }

#define DEC_TEN     \
  (s_decimal) {   \
    { 10, 0, 0, 0 } \
  }

#define DEC_MAX                               \
  (s_decimal) {                             \
    { 4294967295, 4294967295, 4294967295, 0 } \
  }

#define DEC_MIN                                        \
  (s_decimal) {                                      \
    { 4294967295, 4294967295, 4294967295, 2147483648 } \
  }

#define DEC_MINT            \
  (s_decimal) {           \
    { 2147483647, 0, 0, 0 } \
  }

#define DEC_mINT                     \
  (s_decimal) {                    \
    { 2147483648, 0, 0, 2147483648 } \
  }

typedef struct {
  unsigned bits[7];
} s_big_decimal;

#define big_dec_ten          \
  (s_big_decimal) {        \
    { 10, 0, 0, 0, 0, 0, 0 } \
  }

#define big_dec_zero \
  (s_big_decimal) { 0 }

#define big_dec_one         \
  (s_big_decimal) {       \
    { 1, 0, 0, 0, 0, 0, 0 } \
  }

#define MAX_lim_float 7.9228162514264337593543950335e28  // 0x6f800000
#define MIN_lim_float 1e-28
#define INF               \
  (s_decimal) {         \
    { 0, 0, 0, 16711680 } \
  }
#define N_INF               \
  (s_decimal) {           \
    { 0, 0, 0, 2164195328 } \
  }

/* Arithmetic */
int s_add(s_decimal value_1, s_decimal value_2, s_decimal *result);
int s_sub(s_decimal value_1, s_decimal value_2, s_decimal *result);
int s_mul(s_decimal value_1, s_decimal value_2, s_decimal *result);
int s_div(s_decimal value_1, s_decimal value_2, s_decimal *result);
int s_mod(s_decimal value_1, s_decimal value_2, s_decimal *result);
int s_shift(s_big_decimal *value, int step);
int s_find_nonzero_bit(s_big_decimal src);
int s_simple_add(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result);
int s_simple_sub(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result);
int s_simple_mul(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result);
int s_simple_div(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result);
s_big_decimal div_int(s_big_decimal value_1, s_big_decimal value_2,
                        s_big_decimal *result);
int div_fractional(s_big_decimal decimal_reminder, s_big_decimal value_2,
                   s_big_decimal *result);
int s_simple_mod(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result);
int trim_fraction(s_decimal *value);

/* Comparisons */
int s_is_less(s_decimal val1, s_decimal val2);
int s_is_less_or_equal(s_decimal val1, s_decimal val2);
int s_is_greater(s_decimal val1, s_decimal val2);
int s_is_greater_or_equal(s_decimal val1, s_decimal val2);
int s_is_equal(s_decimal val1, s_decimal val2);
int s_is_not_equal(s_decimal val1, s_decimal val2);
int s_is_greater_big(s_big_decimal value_1, s_big_decimal value_2);
int s_is_greater_or_equal_big(s_big_decimal value_1,
                                s_big_decimal value_2);
int s_is_equal_big(s_big_decimal value_1, s_big_decimal value_2);

/* Converters */
int s_from_int_to_decimal(int src, s_decimal *dst);
int s_from_decimal_to_int(s_decimal src, int *dst);
int s_from_float_to_decimal(float src, s_decimal *dst);
int s_from_decimal_to_float(s_decimal src, float *dst);

/* Math funcs */
int s_floor(s_decimal value, s_decimal *result);
int s_round(s_decimal value, s_decimal *result);
int s_truncate(s_decimal value, s_decimal *result);
int s_negate(s_decimal value, s_decimal *result);

/* Additional funcs*/
int s_get_bit(int value, int index);
int s_set_bit(int value, int index);
void s_set_accur(s_decimal *val, int acc);
void s_print_bit(s_decimal value);
int s_zoom(s_decimal value);
int check_zero(s_big_decimal *value);
void set_bit_dec(s_decimal *src, int pos);
void set_bit_big_dec(s_big_decimal *src, int pos);
int check_bit_big_dec(s_big_decimal src, int pos);
void set_scale_big_dec(s_big_decimal *src, int scale);
int get_scale_big_dec(s_big_decimal src);
void equalize_scale(s_big_decimal *value_1, s_big_decimal *value_2);
void reset_big_decimal(s_big_decimal *src);
void reset_decimal(s_decimal *src);
void set_sign_big_dec(s_big_decimal *src, int sign);
void set_sign_dec(s_decimal *src, int sign);
int get_sign_dec(s_decimal src);
int get_sign_big_dec(s_big_decimal src);
int from_dec_to_big(s_decimal src, s_big_decimal *dst);
int from_big_to_dec(s_big_decimal src, s_decimal *dst);
int l_shift(s_decimal *dec, int step);
int r_shift(s_decimal *dec, int step);
void check_zero_dec(s_decimal *src);
int if_dec_inf_or_nan(s_decimal *src, int *flag_err);

#endif  //  SRC_s_DECIMAL_H_  //  SRC_s_DECIMAL_H_
