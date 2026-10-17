/**
 ******************************************************************************
 * @file    ConvertCalendar.h
 * @brief   Jalali (Persian) <-> Gregorian calendar conversion.
 *
 * @details This module provides robust, bidirectional conversion.
 *          It is designed to be 100% drop-in compatible with legacy code.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#ifndef JALALI_CALENDAR_H
#define JALALI_CALENDAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Structure representing a calendar date.
 * @note   Uses `int` (instead of int32_t) to ensure seamless compatibility
 *         with standard `snprintf` (%d) formatting in the main application.
 */
typedef struct
{
    int year;
    int month;
    int day;
} date;

/**
 * @brief  Convert a Gregorian date to a Jalali (Persian) date.
 * @param  input  Gregorian date.
 * @return Corresponding Jalali date (returns zeroed date if input is invalid).
 */
date gregorian2jalali(date input);

/**
 * @brief  Convert a Jalali (Persian) date to a Gregorian date.
 * @param  input  Jalali date.
 * @return Corresponding Gregorian date (returns zeroed date if input is invalid).
 */
date jalali2gregorian(date input);

#ifdef __cplusplus
}
#endif

#endif /* JALALI_CALENDAR_H */
