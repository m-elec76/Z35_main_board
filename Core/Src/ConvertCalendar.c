/**
 ******************************************************************************
 * @file    ConvertCalendar.c
 * @brief   Jalali (Persian) <-> Gregorian calendar conversion.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#include "ConvertCalendar.h"

#define JALALI_MIN_YEAR 1
#define JALALI_MAX_YEAR 3177
#define JDN_JALALI_EPOCH 1948320

typedef struct
{
    int leap;
    int gregorian_year;
    int march_day;
} JalaliYearInfo;

static const int jalali_breaks[] =
{
    -61, 9, 38, 199, 426, 686, 756, 818, 1111, 1181,
    1210, 1635, 2060, 2097, 2192, 2262, 2324, 2394,
    2456, 3178
};

#define BREAK_COUNT \
    ((int)(sizeof(jalali_breaks) / sizeof(jalali_breaks[0])))

/*----------------------------------------------------------
 * Gregorian helpers
 *----------------------------------------------------------*/

static bool IsGregorianLeapYear(int year)
{
    return ((year % 4 == 0) &&
            ((year % 100 != 0) || (year % 400 == 0)));
}

static bool IsValidGregorianDate(date input)
{
    static const int month_days[12] =
    {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    int max_day;

    if (input.month < 1 || input.month > 12) return false;
    if (input.day < 1) return false;

    max_day = month_days[input.month - 1];

    if (input.month == 2 && IsGregorianLeapYear(input.year))
        max_day = 29;

    return input.day <= max_day;
}

static int GregorianToJDN(int gy, int gm, int gd)
{
    int a = (14 - gm) / 12;
    int y = gy + 4800 - a;
    int m = gm + 12 * a - 3;

    return gd + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
}

static void JDNToGregorian(int jdn, int *gy, int *gm, int *gd)
{
    int a = jdn + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - (146097 * b) / 4;
    int d = (4 * c + 3) / 1461;
    int e = c - (1461 * d) / 4;
    int m = (5 * e + 2) / 153;

    *gd = e - (153 * m + 2) / 5 + 1;
    *gm = m + 3 - 12 * (m / 10);
    *gy = 100 * b + d - 4800 + m / 10;
}

/*----------------------------------------------------------
 * Jalali year calculations
 *----------------------------------------------------------*/

static bool JalaliCalendar(int jy, JalaliYearInfo *info)
{
    int gy, leap_j, leap_g, jp, jm, jump, n, leap, i;

    if (info == 0) return false;
    if (jy < JALALI_MIN_YEAR || jy > JALALI_MAX_YEAR) return false;

    leap_j = -14;
    jp = jalali_breaks[0];
    jump = 0;

    for (i = 1; i < BREAK_COUNT; i++)
    {
        jm = jalali_breaks[i];
        jump = jm - jp;

        if (jy < jm) break;

        leap_j += (jump / 33) * 8 + (jump % 33) / 4;
        jp = jm;
    }

    if (i >= BREAK_COUNT) return false;

    n = jy - jp;
    leap_j += (n / 33) * 8 + ((n % 33) + 3) / 4;

    if ((jump % 33 == 4) && (jump - n == 4)) leap_j++;

    gy = jy + 621;
    leap_g = gy / 4 - (((gy / 100) + 1) * 3) / 4 - 150;

    info->gregorian_year = gy;
    info->march_day = 20 + leap_j - leap_g;

    if (jump - n < 6) n = n - jump + ((jump + 4) / 33) * 33;

    leap = ((n + 1) % 33 - 1) % 4;
    if (leap == -1) leap = 4;

    info->leap = leap;
    return true;
}

static bool IsValidJalaliDate(date input)
{
    JalaliYearInfo info;
    int max_day;

    if (input.year < JALALI_MIN_YEAR || input.year > JALALI_MAX_YEAR) return false;
    if (input.month < 1 || input.month > 12) return false;
    if (input.day < 1) return false;

    if (!JalaliCalendar(input.year, &info)) return false;

    if (input.month <= 6) max_day = 31;
    else if (input.month <= 11) max_day = 30;
    else max_day = (info.leap == 0) ? 30 : 29;

    return input.day <= max_day;
}

/*----------------------------------------------------------
 * Jalali <-> JDN
 *----------------------------------------------------------*/

static int JalaliToJDN(int jy, int jm, int jd)
{
    JalaliYearInfo info;
    int jdn;

    (void)JalaliCalendar(jy, &info);

    jdn = GregorianToJDN(info.gregorian_year, 3, info.march_day);

    if (jm <= 7) jdn += (jm - 1) * 31;
    else jdn += 186 + (jm - 7) * 30;

    jdn += jd - 1;
    return jdn;
}

static bool JDNToJalali(int jdn, int *jy, int *jm, int *jd)
{
    JalaliYearInfo info;
    int gy, gm, gd, year, first_day, k;

    JDNToGregorian(jdn, &gy, &gm, &gd);

    year = gy - 621;

    if (year > JALALI_MAX_YEAR) return false;
    if (year < JALALI_MIN_YEAR) return false;
    if (!JalaliCalendar(year, &info)) return false;

    first_day = GregorianToJDN(gy, 3, info.march_day);
    k = jdn - first_day;

    if (k >= 0)
    {
        if (k <= 185)
        {
            *jy = year;
            *jm = 1 + k / 31;
            *jd = 1 + k % 31;
            return true;
        }
        k -= 186;
    }
    else
    {
        year--;
        if (year < JALALI_MIN_YEAR) return false;
        k += 179;
        if (info.leap == 1) k++;
    }

    *jy = year;
    *jm = 7 + k / 30;
    *jd = 1 + k % 30;

    return true;
}

/*----------------------------------------------------------
 * Public API (Legacy-compatible)
 *----------------------------------------------------------*/

date gregorian2jalali(date input)
{
    int jdn;
    date result = {0, 0, 0};

    if (!IsValidGregorianDate(input)) return result;

    jdn = GregorianToJDN(input.year, input.month, input.day);

    if (!JDNToJalali(jdn, &result.year, &result.month, &result.day))
    {
        result.year = 0; result.month = 0; result.day = 0;
    }

    return result;
}

date jalali2gregorian(date input)
{
    int jdn;
    date result = {0, 0, 0};

    if (!IsValidJalaliDate(input)) return result;

    jdn = JalaliToJDN(input.year, input.month, input.day);
    JDNToGregorian(jdn, &result.year, &result.month, &result.day);

    return result;
}
