#include "s_decimal.h"

/* Из инта в децимал */
int s_from_int_to_decimal(int src, s_decimal *dst) {
  *dst = DEC_NULL;
  int flag_err = 0;
  int residue = 0;
  int count = 0;
  if (&src != NULL) {
    if (src < 0) {
      dst->bits[3] = s_set_bit(dst->bits[3], 31);
      src *= (-1);
    }
    dst->bits[0] = src;
  } else {
    flag_err = 1;
  }
  return flag_err;
}

/* Из децимала в инт */
int s_from_decimal_to_int(s_decimal src, int *dst) {
  *dst = 0;
  int flag_err = 0;
  s_decimal src_t = {0};
  s_truncate(src, &src_t);
  if (s_is_greater_or_equal(src_t, DEC_mINT) &&
      s_is_less_or_equal(src_t, DEC_MINT)) {
    *dst = src_t.bits[0];
    if (s_get_bit(src_t.bits[3], 31)) (*dst) *= (-1);
  } else {
    flag_err = 1;
  }
  return flag_err;
}

/* Из флоата в децимал */
int s_from_float_to_decimal(float src, s_decimal *dst) {
  *dst = DEC_NULL;
  int flag_err = 0, exp = 0;
  int bit = 0, scale = 0;
  unsigned fbits_src = 0;

  if (isnan(src) || isinf(src) || src <= MAX_lim_float * (-1) ||
      src >= MAX_lim_float ||
      (src > (-1) * MIN_lim_float && src < MIN_lim_float && src != 0)) {
    flag_err = 1;
  } else {
    if (src != 0) {
      if (src < 0) {
        dst->bits[3] = s_set_bit(dst->bits[3], 31);
        src = -src;
      }
      /* вычленение экспоненты */
      fbits_src = *((unsigned int *)&src);
      for (int i = 23, j = 0; i <= 30; i++, j++) {
        bit = s_get_bit(fbits_src, i);
        if (bit) exp = s_set_bit(exp, j);
      }
      exp -= 127;
      if (exp > 96) {  // 95 //максим число 79228162514264337593543950335.0
                       // экспонента 96
        flag_err = 1;
      } else {
        float fresidue = 2;
        int count_number = 0;
        int shift = 0;
        if (exp < 0) {
          //нормализация
          for (int i = 0; i < 6 && !((int)src); src *= 10, scale++, i++) {
          }
        } else {
          //значимые цифры
          fresidue = src;
          while (fresidue > 1) {
            fresidue /= 10;
            count_number++;
          }
          if (count_number < 7) {
            int del = 1000000;
            for (fresidue = 0; fresidue < 1 && count_number < 7;
                 src *= 10, count_number++, shift++) {
              fresidue = src / del;
            }
          }
        }
        //округление
        src = round(src);
        scale += shift;

        if (exp < -94 || scale > 28) {  //-94 это экспонента мин числа 1e-28
          flag_err = 1;
        } else {
          fbits_src = 0;
          fbits_src = *((unsigned int *)&src);
          exp = 0;
          for (int i = 23, j = 0; i <= 30; i++, j++) {
            bit = s_get_bit(fbits_src, i);
            if (bit) exp = s_set_bit(exp, j);
          }
          exp -= 127;
          int residue = 0;
          int count = 0;
          while (src > 0) {
            residue = fmodf(src, 2);
            src /= 2;
            if (residue) {
              if (count < 32) {
                dst->bits[0] = s_set_bit(dst->bits[0], count);
              } else if (count >= 32 && count < 64) {
                dst->bits[1] = s_set_bit(dst->bits[1], count - 32);
              } else if (count >= 64 && count < 96) {
                dst->bits[2] = s_set_bit(dst->bits[2], count - 64);
              }
            }
            count++;
          }
          s_set_accur(dst, scale);
          s_decimal res;
          s_div(*dst, DEC_1, &res);
          *dst = res;
        }
      }
    }
  }
  return flag_err;
}

/* Из децимала во флоат */
int s_from_decimal_to_float(s_decimal src, float *dst) {
  *dst = 0;
  double ret = 0;
  int flag_err = 0, bit = 0, scale = 0;
  int eq_inf = 0, eq_n_inf = 0;
  flag_err = if_dec_inf_or_nan(&src, &flag_err);

  if (flag_err != 0) {
    flag_err = 1;
  } else {
    // взятие числа
    for (int j = 0; j < 3; j++) {
      for (int i = 0; i < 32; i++) {
        bit = s_get_bit(src.bits[j], i);
        if (bit) ret += pow(2, i + j * 32);
      }
    }
    //взятие масштаба
    scale = s_zoom(src);
    if (scale >= 0 && scale <= 28) {
      for (; scale > 0; scale--) ret /= 10;
    } else {
      flag_err = 1;
    }
    //знак
    bit = s_get_bit(src.bits[3], 31);
    if (bit) ret = -(ret);
    *dst = ret;
  }
  return flag_err;
}

int if_dec_inf_or_nan(s_decimal *src, int *flag_err) {
  *flag_err = 0;
  int bit = 0;
  int exp = 0;
  for (int i = 16, j = 0; i < 24; i++, j++) {
    bit = s_get_bit(src->bits[3], i);
    if (bit) exp = s_set_bit(exp, j);
  }
  if (exp == 255) {
    *flag_err = 1;
    if (bit = s_get_bit(src->bits[3], 31)) *flag_err = 2;
    bit = 0;
    for (int i = 0; i < 96 && !bit; i++) {
      if (i < 32) {
        bit = s_get_bit(src->bits[0], i);
      } else if (i >= 32 && i < 64) {
        bit = s_get_bit(src->bits[1], i);
      } else if (i >= 64 && i < 96) {
        bit = s_get_bit(src->bits[2], i);
      }
    }
    if (bit) *flag_err = 3;
  } else {
    *flag_err = 0;
  }
  return *flag_err;
}
