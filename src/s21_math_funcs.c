#include "s_decimal.h"

/* Округление в меньшую сторону */
int s_floor(s_decimal value, s_decimal *result) {
  *result = DEC_NULL;
  int status = 0;
  int scale = s_zoom(value);
  int sign = s_get_bit(value.bits[3], 31);
  *result = value;
  if (scale) {
    int zero = 1;
    trim_fraction(result);
    for (int i = 0; i < 3 && zero; i++) zero = value.bits[i] == 0;
    if (sign && !zero) {
      s_negate(*result, result);
      s_add(*result, DEC_1, result);
      s_negate(*result, result);
    }
  }
  check_zero_dec(result);
  return status;
}

/* Математическое округление */
int s_round(s_decimal value,
              s_decimal *result) { /* получение остатка от 0,1 */
  *result = DEC_NULL;
  int error = 0;
  s_decimal save_res = {0};
  error = s_mod(value, DEC_1, &save_res); /* получаем десятую часть */
  if (!error) error = s_mul(save_res, DEC_TEN, &save_res);
  if (!error) error = s_truncate(save_res, &save_res);
  if (s_get_bit(save_res.bits[3], 31)) s_negate(save_res, &save_res);
  if (s_is_less(DEC_4, save_res) &&
      !s_get_bit(value.bits[3], 31)) { /* прибавлять */
    if (!error) error = s_add(value, DEC_1, &value);
    if (!error) error = s_truncate(value, &(*result));
  } else if (s_is_less(DEC_4, save_res) && s_get_bit(value.bits[3], 31)) {
    if (!error) s_sub(value, DEC_1, &value);
    if (!error) s_truncate(value, &(*result));
  } else {
    if (!error) s_truncate(value, &(*result));
  }
  check_zero_dec(result);
  return !!error;
}

/* Взятие целой части */
int s_truncate(s_decimal value, s_decimal *result) {
  *result = DEC_NULL;
  int error = 0;
  int zoom = s_zoom(value); /* берём зум */
  s_decimal zoom_dec = {0}; /* for mod */
  zoom_dec.bits[0] = 1;
  s_decimal result_mod = value;                     /* for mod */
  s_decimal result_mod_end = {0};                   /* for mod */
  error = s_mod(result_mod, zoom_dec, &result_mod); /* записываем */
  if (!error)
    error = s_sub(value, result_mod, &(result_mod_end)); /* вычитаем*/
  if (!error) {
    s_decimal tmp = {0};
    error = s_div(result_mod_end, zoom_dec, result);
  }
  return !!error;
}

/* Умножение на (-1) */
int s_negate(s_decimal value, s_decimal *result) {
  int error = 0;
  value.bits[3] ^= (unsigned)pow(2, 31);
  *result = value;
  return error;
}
