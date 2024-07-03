#include "s_decimal.h"

/* Сложение */
int s_add(s_decimal value_1, s_decimal value_2, s_decimal *result) {
  *result = DEC_NULL;
  s_big_decimal tmp_1 = {0}, tmp_2 = {0}, tmp_res = {0};
  reset_decimal(result);
  from_dec_to_big(value_1, &tmp_1);
  from_dec_to_big(value_2, &tmp_2);
  check_zero(&tmp_1);
  check_zero(&tmp_2);

  int sign_1 = get_sign_big_dec(tmp_1);
  int sign_2 = get_sign_big_dec(tmp_2);
  set_sign_big_dec(&tmp_1, 0);
  set_sign_big_dec(&tmp_2, 0);

  equalize_scale(&tmp_1, &tmp_2);

  if (sign_1 == sign_2) {
    s_simple_add(tmp_1, tmp_2, &tmp_res);
    set_sign_big_dec(&tmp_res, sign_1);
  } else {
    if (s_is_greater_or_equal_big(tmp_1, tmp_2)) {
      s_simple_sub(tmp_1, tmp_2, &tmp_res);
      set_sign_big_dec(&tmp_res, sign_1);
    } else {
      s_simple_sub(tmp_2, tmp_1, &tmp_res);
      set_sign_big_dec(&tmp_res, sign_2);
    }
  }
  set_scale_big_dec(&tmp_res, get_scale_big_dec(tmp_1));
  int error_status = from_big_to_dec(tmp_res, result);
  check_zero_dec(result);
  return error_status;
}

/* Вычитание */
int s_sub(s_decimal value_1, s_decimal value_2, s_decimal *result) {
  *result = DEC_NULL;
  s_big_decimal tmp_1 = {0}, tmp_2 = {0}, tmp_res = {0};
  from_dec_to_big(value_1, &tmp_1);
  from_dec_to_big(value_2, &tmp_2);
  check_zero(&tmp_1);
  check_zero(&tmp_2);
  int sign_1 = get_sign_big_dec(tmp_1);
  int sign_2 = get_sign_big_dec(tmp_2);
  reset_decimal(result);

  equalize_scale(&tmp_1, &tmp_2);
  int sign = sign_1 + sign_2;
  int scale = get_scale_big_dec(tmp_1);
  if (sign == 1) {
    s_simple_add(tmp_1, tmp_2, &tmp_res);
    set_sign_big_dec(&tmp_res, sign_1);
  } else {
    if (s_is_greater_or_equal_big(tmp_1, tmp_2)) {
      s_simple_sub(tmp_1, tmp_2, &tmp_res);
      if (sign == 2) {
        set_sign_big_dec(&tmp_res, NEGATIVE_SIGN);
      }
    } else {
      s_simple_sub(tmp_2, tmp_1, &tmp_res);
      if (sign != 2) {
        set_sign_big_dec(&tmp_res, NEGATIVE_SIGN);
      }
    }
  }
  set_scale_big_dec(&tmp_res, scale);

  int error_status = from_big_to_dec(tmp_res, result);
  check_zero_dec(result);
  return error_status;
}

/* Умножение */
int s_mul(s_decimal value_1, s_decimal value_2, s_decimal *result) {
  *result = DEC_NULL;
  s_big_decimal tmp_1 = {0}, tmp_2 = {0}, tmp_res = {0};
  from_dec_to_big(value_1, &tmp_1);
  from_dec_to_big(value_2, &tmp_2);
  reset_decimal(result);

  int sign = get_sign_dec(value_1) + get_sign_dec(value_2);
  int scale = s_zoom(value_1) + s_zoom(value_2);
  int zero = check_zero(&tmp_1) || check_zero(&tmp_2);

  tmp_1.bits[6] = 0;
  tmp_2.bits[6] = 0;

  if (!zero) {
    s_simple_mul(tmp_1, tmp_2, &tmp_res);
    set_scale_big_dec(&tmp_res, scale);
  }

  if (sign == 1)
    set_sign_big_dec(&tmp_res, NEGATIVE_SIGN);
  else
    set_sign_big_dec(&tmp_res, POSITIVE_SIGN);
  int error_status = from_big_to_dec(tmp_res, result);
  check_zero_dec(result);
  return error_status;
}

/* Деление */
int s_div(s_decimal value_1, s_decimal value_2, s_decimal *result) {
  *result = DEC_NULL;
  s_big_decimal tmp_1 = {0}, tmp_2 = {0}, tmp_res = {0};
  from_dec_to_big(value_1, &tmp_1);
  from_dec_to_big(value_2, &tmp_2);
  reset_decimal(result);

  int error_status = s_simple_div(tmp_1, tmp_2, &tmp_res);
  if (error_status == 0) {
    error_status = from_big_to_dec(tmp_res, result);
  }
  check_zero_dec(result);
  return error_status;
}

/* Взятие остатка от деления v1 на v2 */
int s_mod(s_decimal value_1, s_decimal value_2, s_decimal *result) {
  *result = DEC_NULL;
  int error = 0;
  char minus = 0;
  if (s_get_bit(value_1.bits[3], 31)) {
    minus = 1;
    s_negate(value_1, &value_1);
  }
  if (s_get_bit(value_2.bits[3], 31)) s_negate(value_2, &value_2);
  if (s_is_equal(value_2, DEC_NULL)) {
    error = 3;
  } else if (s_is_equal(value_1, DEC_NULL) ||
             s_is_equal(value_1, value_2)) {
    *result = DEC_NULL;
  } else if (s_is_less(value_1, value_2)) {
    if (minus == 1) s_negate(value_1, &value_1);
    *result = value_1;
  } else {
    s_decimal mod = value_2;
    for (int i = 1;; i++) { /* Сдвиг v2 влево к v1 */
      int over = l_shift(&mod, 1);
      if (s_is_equal(mod, value_1)) { /* v2 равен v1 после сдвигов */
        *result = DEC_NULL;
        break;
      } else if (s_is_greater(mod, value_1) || over) {
        if (!over)
          r_shift(&mod, 1); /* сдвиг v2 вправо если не было переполнения */
        while (s_is_greater(value_1, mod)) {
          s_decimal tmp_sub = {0};
          s_sub(value_1, mod, &tmp_sub);
          value_1 = tmp_sub;
          while (s_is_greater(mod, value_1)) {
            if (r_shift(&mod, 1)) break;
            if (s_is_less(mod, value_2)) {
              l_shift(&mod, 1);
              break;
            }
          }
        }
        while (s_is_less(mod, value_1)) {
          s_decimal tmp_sum = {0};
          error = s_add(mod, value_2, &tmp_sum);
          if (error > 0) break;
          mod = tmp_sum;
        }
        if (s_is_greater(mod, value_1) && !error) {
          s_decimal tmp_sum = {0}, tmp_sum2 = {0};
          error = s_sub(mod, value_2, &tmp_sum);
          if (!error) error = s_sub(value_1, tmp_sum, &tmp_sum2);
          mod = tmp_sum2;
        } else if (s_is_equal(mod, value_1)) {
          mod = DEC_NULL;
        }
        if (minus == 1) s_negate(mod, &mod);
        *result = mod;
        break;
      }
    }
  }
  check_zero_dec(result);
  return error;
}

/* Сдвиг биг децимала */
int s_shift(s_big_decimal *value, int step) {
  if (step > 0) {
    for (int i = 0; i < step; i++) {
      int zero = check_bit_big_dec(*value, 31);
      int one = check_bit_big_dec(*value, 63);
      int two = check_bit_big_dec(*value, 95);
      int three = check_bit_big_dec(*value, 127);
      int four = check_bit_big_dec(*value, 159);
      for (int i = 0; i < 6; i++) {
        value->bits[i] <<= 1;
      }
      if (zero) set_bit_big_dec(value, 32);
      if (one) set_bit_big_dec(value, 64);
      if (two) set_bit_big_dec(value, 96);
      if (three) set_bit_big_dec(value, 128);
      if (four) set_bit_big_dec(value, 160);
    }
  } else {
    for (int i = 0; i < (-1) * step; i++) {
      int zero = check_bit_big_dec(*value, 32);
      int one = check_bit_big_dec(*value, 64);
      int two = check_bit_big_dec(*value, 96);
      int three = check_bit_big_dec(*value, 128);
      int four = check_bit_big_dec(*value, 160);
      for (int i = 0; i < 6; i++) {
        value->bits[i] >>= 1;
      }
      if (zero) set_bit_big_dec(value, 31);
      if (one) set_bit_big_dec(value, 63);
      if (two) set_bit_big_dec(value, 95);
      if (three) set_bit_big_dec(value, 127);
      if (four) set_bit_big_dec(value, 159);
    }
  }
  return 0;
}

int s_find_nonzero_bit(s_big_decimal src) {
  int result = 191;

  while (result > 0 && !check_bit_big_dec(src, result)) result--;

  return result;
}

/* Сложение биг децимала */
int s_simple_add(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result) {
  int reminder = 0;
  reset_big_decimal(result);
  for (int i = 0; i <= 191; i++) {
    int sum = check_bit_big_dec(value_1, i) + check_bit_big_dec(value_2, i) +
              reminder;
    reminder = sum > 1 ? 1 : 0;
    if (sum == 3 || sum == 1) set_bit_big_dec(result, i);
  }
  check_zero(result);
  return reminder;
}

/* Вычитание Биг децимала */
int s_simple_sub(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result) {
  int error_status = 0;
  int carry = 0;
  int diff = 0;
  int left_bit = 0;
  int right_bit = 0;

  reset_big_decimal(result);

  for (int i = 0; i < 192; i++) {
    left_bit = check_bit_big_dec(value_1, i);
    right_bit = check_bit_big_dec(value_2, i);
    diff = left_bit - right_bit;
    if (!carry) {
      if (diff) {
        set_bit_big_dec(result, i);
        if (diff < 0) carry = 1;
      }
    } else {
      diff--;
      if (!diff) {
        carry = 0;
      } else if (diff == -1) {
        set_bit_big_dec(result, i);
      }
    }
  }
  check_zero(result);
  return error_status;
}

/* Умножение биг децимала */
int s_simple_mul(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result) {
  s_big_decimal tmp = {0};
  int error_status = 0;
  int shift_step = 0;
  reset_big_decimal(result);
  int stop = s_find_nonzero_bit(value_2);

  for (int idx = 0; idx <= stop; idx++) {
    if (check_bit_big_dec(value_2, idx)) {
      tmp = value_1;
      s_shift(&tmp, shift_step);
      s_simple_add(*result, tmp, result);
    }
    shift_step++;
  }
  return error_status;
}

/* Деление биг децимала */
int s_simple_div(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result) {
  int error_status = 0;

  int first_sign = get_sign_big_dec(value_1);
  int second_sign = get_sign_big_dec(value_2);

  set_sign_big_dec(&value_1, POSITIVE_SIGN);
  set_sign_big_dec(&value_2, POSITIVE_SIGN);
  equalize_scale(&value_1, &value_2);

  if (!s_is_equal_big(big_dec_zero, value_2)) {
    div_fractional(div_int(value_1, value_2, result), value_2, result);
  } else {
    error_status = 3;
  }

  if ((first_sign && !second_sign) || (!first_sign && second_sign))
    set_sign_big_dec(result, NEGATIVE_SIGN);
  else
    set_sign_big_dec(result, POSITIVE_SIGN);

  return error_status;
}

s_big_decimal div_int(s_big_decimal value_1, s_big_decimal value_2,
                        s_big_decimal *result) {
  reset_big_decimal(result);

  int start_idx = s_find_nonzero_bit(value_1);
  int shift_step = start_idx - s_find_nonzero_bit(value_2);

  value_1.bits[6] = 0;
  value_2.bits[6] = 0;

  check_zero(result);
  s_shift(&value_2, shift_step);

  while (shift_step >= 0) {
    s_shift(result, 1);
    if (s_is_greater_or_equal_big(value_1, value_2)) {
      set_bit_big_dec(result, 0);
      s_simple_sub(value_1, value_2, &value_1);
    }
    s_shift(&value_2, -1);

    shift_step--;
  }

  return value_1;
}

int div_fractional(s_big_decimal decimal_reminder, s_big_decimal value_2,
                   s_big_decimal *result) {
  int error_status = 0;
  s_big_decimal decimal_buffer = {0};

  unsigned scale = 0;
  while (!s_is_equal_big(decimal_reminder, big_dec_zero) && (scale < 29)) {
    s_simple_mul(big_dec_ten, decimal_reminder, &decimal_reminder);
    s_simple_mul(*result, big_dec_ten, result);
    scale++;

    if (s_is_greater_or_equal_big(decimal_reminder, value_2)) {
      decimal_reminder = div_int(decimal_reminder, value_2, &decimal_buffer);
      s_simple_add(decimal_buffer, *result, result);
    }
  }
  set_scale_big_dec(result, scale);
  return error_status;
}

int trim_fraction(s_decimal *value) {
  s_big_decimal big_value = {0};
  s_big_decimal last_num = {0};
  from_dec_to_big(*value, &big_value);
  int sign = get_sign_big_dec(big_value);
  int zero = check_zero(&big_value);
  if (!zero) {
    int scale = get_scale_big_dec(big_value);
    while (scale) {
      if (scale == 1) s_simple_mod(big_value, big_dec_ten, &last_num);
      div_int(big_value, big_dec_ten, &big_value);
      scale--;
    }
  }
  set_sign_big_dec(&big_value, sign);
  set_scale_big_dec(&big_value, 0);
  from_big_to_dec(big_value, value);
  return last_num.bits[0];
}

/* Остаток от деления биг децимала */
int s_simple_mod(s_big_decimal value_1, s_big_decimal value_2,
                   s_big_decimal *result) {
  int error_status = 0;
  s_big_decimal decimal_buffer = {0};

  int first_sign = get_sign_big_dec(value_1);
  int second_sign = get_sign_big_dec(value_2);
  set_sign_big_dec(&value_1, POSITIVE_SIGN);
  set_sign_big_dec(&value_2, POSITIVE_SIGN);
  // equalize_scale(&value_1, &value_2);

  if (!s_is_equal_big(decimal_buffer, value_2)) {
    int sign = ((get_sign_big_dec(value_1) + get_sign_big_dec(value_2)) % 2);
    set_sign_big_dec(&value_1, POSITIVE_SIGN);
    set_sign_big_dec(&value_2, POSITIVE_SIGN);
    *result = div_int(value_1, value_2, &decimal_buffer);
    if (sign) set_sign_big_dec(result, NEGATIVE_SIGN);
  } else {
    error_status = 3;
  }

  if (first_sign == 1 || second_sign == 1)
    set_sign_big_dec(result, NEGATIVE_SIGN);
  else
    set_sign_big_dec(result, POSITIVE_SIGN);
  check_zero(result);
  return error_status;
}
