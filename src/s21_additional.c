#include "s_decimal.h"

int s_get_bit(int value, int index) { return !!(value & (1 << index)); }

int s_set_bit(int value, int index) { return (value | (1 << index)); }

/* Вставка точности */
void s_set_accur(s_decimal *val, int acc) {
  if (acc >= 0 && acc < 29) {
    for (int i = 16; i < 21; i++)
      if (s_get_bit(val->bits[3], i)) val->bits[3] ^= (unsigned)pow(2, i);
    for (int i = 0; i < 5; i++)
      if (s_get_bit(acc, i)) val->bits[3] = s_set_bit(val->bits[3], 16 + i);
  }
}

/* Вывод битов децимала */
void s_print_bit(s_decimal value) {
  for (int j = 3; j >= 0; j--) {
    for (int i = 31; i >= 0; i--) {
      printf("%d", s_get_bit(value.bits[j], i));
    }
    printf("|");
  }
  puts("");
}

/* Взятие точности */
int s_zoom(s_decimal value) {
  int acc = 0;
  for (int i = 16; i < 21; i++)
    if (s_get_bit(value.bits[3], i)) acc = s_set_bit(acc, i - 16);
  if (acc > 28) acc = 28;
  return acc;
}

int from_dec_to_big(s_decimal src, s_big_decimal *dst) {
  dst->bits[0] = src.bits[0];
  dst->bits[1] = src.bits[1];
  dst->bits[2] = src.bits[2];
  dst->bits[6] = src.bits[3];
  return 0;
}

int from_big_to_dec(s_big_decimal src, s_decimal *dst) {
  int error_code = 0;
  s_big_decimal last_num = {0};
  int big_scale = get_scale_big_dec(src);
  int sign = get_sign_big_dec(src);
  reset_decimal(dst);

  while ((((src.bits[5] || src.bits[4] || src.bits[3]) && big_scale) ||
          (big_scale > 28)) &&
         !error_code) {
    last_num = div_int(src, big_dec_ten, &src);
    big_scale--;

    if ((big_scale > 28) && !src.bits[5] && !src.bits[4] && !src.bits[3] &&
        !src.bits[2] && !src.bits[1] && !src.bits[0])
      error_code = 2;
  }
  if (error_code != 2) {
    if (last_num.bits[0] >= 5) s_simple_add(big_dec_one, src, &src);
    if (src.bits[5] || src.bits[4] || src.bits[3]) {
      if (sign == NEGATIVE_SIGN)
        error_code = 2;
      else
        error_code = 1;
    } else {
      for (int i = 0; i < 3; i++) dst->bits[i] = src.bits[i];
      s_set_accur(dst, big_scale);
      set_sign_dec(dst, sign);
    }
  }
  return error_code;
}

int get_sign_big_dec(s_big_decimal src) {
  return !!(src.bits[6] & 2147483648);
}

int get_sign_dec(s_decimal src) { return !!(src.bits[3] & 2147483648); }

void set_sign_dec(s_decimal *src, int sign) {
  if (sign) {
    set_bit_dec(src, 127);
  } else {
    src->bits[3] &= ~2147483648;
  }
}

void set_sign_big_dec(s_big_decimal *src, int sign) {
  if (sign) {
    set_bit_big_dec(src, 223);
  } else {
    src->bits[6] &= ~2147483648;
  }
}

void reset_decimal(s_decimal *src) {
  for (int i = 0; i < 4; i++) src->bits[i] = 0;
}

void reset_big_decimal(s_big_decimal *src) {
  for (int i = 0; i < 7; i++) src->bits[i] = 0;
}

void equalize_scale(s_big_decimal *value_1, s_big_decimal *value_2) {
  int scale1 = get_scale_big_dec(*value_1);
  int scale2 = get_scale_big_dec(*value_2);
  int sign_1 = get_sign_big_dec(*value_1);
  int sign_2 = get_sign_big_dec(*value_2);

  if (scale1 > scale2) {
    for (int i = scale2; i < scale1; i++)
      s_simple_mul(*value_2, big_dec_ten, value_2);
    set_scale_big_dec(value_2, scale1);
  } else if (scale2 > scale1) {
    for (int i = scale1; i < scale2; i++)
      s_simple_mul(*value_1, big_dec_ten, value_1);
    set_scale_big_dec(value_1, scale2);
  }
  set_sign_big_dec(value_1, sign_1);
  set_sign_big_dec(value_2, sign_2);
}

int get_scale_big_dec(s_big_decimal src) {
  set_sign_big_dec(&src, 0);
  return src.bits[6] >>= 16;
}

void set_scale_big_dec(s_big_decimal *src, int scale) {
  int sign = get_sign_big_dec(*src);
  src->bits[6] = scale <<= 16;
  set_sign_big_dec(src, sign);
}

int check_bit_big_dec(s_big_decimal src, int pos) {
  int dec_idx = pos / 32;
  unsigned mask = 1u << pos % 32;
  return !!(src.bits[dec_idx] & mask);
}

void set_bit_big_dec(s_big_decimal *src, int pos) {
  unsigned mask = 1u << (pos % 32);
  int src_idx = pos / 32;
  src->bits[src_idx] |= mask;
}

int check_zero(s_big_decimal *value) {
  int zero = 1;
  for (int i = 0; i < 6 && zero; i++) zero = value->bits[i] == 0;
  if (zero) set_sign_big_dec(value, POSITIVE_SIGN);
  return zero;
}

void set_bit_dec(s_decimal *src, int pos) {
  unsigned mask = 1u << (pos % 32);
  int src_idx = pos / 32;
  src->bits[src_idx] |= mask;
}

/* Сдвиг влево на step */
int l_shift(s_decimal *dec, int step) {
  int overflow = 0;
  if (step && s_is_not_equal(*dec, DEC_NULL)) {
    for (int i = 2; i >= 0; i--) {
      for (int j = 31; j >= 0; j--) {
        if (s_get_bit(dec->bits[i], j) == 1) {
          if (step + (32 * i) + j > 95) {
            overflow = 1;
            break;
          }
          if (s_get_bit(dec->bits[i], j)) {
            if (j + step > 63) {
              dec->bits[i + 2] = s_set_bit(dec->bits[i + 2], j + step - 64);
            } else if (j + step > 31) {
              dec->bits[i + 1] = s_set_bit(dec->bits[i + 1], j + step - 32);
            } else {
              dec->bits[i] = s_set_bit(dec->bits[i], j + step);
            }
            dec->bits[i] ^= (unsigned)pow(2, j);
          }
        }
      }
      if (overflow) break;
    }
  }
  return overflow;
}

/* Сдвиг вправо на step (без потери единиц) */
int r_shift(s_decimal *dec, int step) {
  int error = 0;
  if (step > 0 && s_is_not_equal(*dec, DEC_NULL)) {
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 32; j++) {
        if (s_get_bit(dec->bits[i], j)) {
          if (32 * i + j - step < 0) {
            error = 1;
            break;
          } else if (j - step >= 0) {
            dec->bits[i] = s_set_bit(dec->bits[i], j - step);
          } else if (j - step >= -32) {
            dec->bits[i - 1] = s_set_bit(dec->bits[i - 1], 32 + j - step);
          } else if (j - step >= -64) {
            dec->bits[i - 2] = s_set_bit(dec->bits[i - 2], 32 + j - step);
          }
          dec->bits[i] ^= (unsigned)pow(2, j);
        }
      }
      if (error) break;
    }
  }
  return error;
}

void check_zero_dec(s_decimal *src) {
  int zero = 1;
  for (int i = 0; i < 3 && zero; i++) zero = src->bits[i] == 0;
  if (zero) src->bits[3] = 0;
}
