#ifndef __UINT_STRINGS_H
#define __UINT_STRINGS_H

// Helper to get character at position N from a string-like sequence
#define CHAR_AT(str, n) ((sizeof(str) > (n)) ? (str)[n] : 0)

#define SWAP_STRING

// Main macro that converts a "string" (as a sequence of characters) to uint16_t array

#ifndef SWAP_STRING
    #define STR_TO_UINT16_ARRAY(str) \
        {((uint16_t)((CHAR_AT(str, 0) << 8) | CHAR_AT(str, 1))), \
         ((uint16_t)((CHAR_AT(str, 2) << 8) | CHAR_AT(str, 3))), \
         ((uint16_t)((CHAR_AT(str, 4) << 8) | CHAR_AT(str, 5))), \
         ((uint16_t)((CHAR_AT(str, 6) << 8) | CHAR_AT(str, 7))), \
         ((uint16_t)((CHAR_AT(str, 8) << 8) | CHAR_AT(str, 9))), \
         ((uint16_t)((CHAR_AT(str, 10) << 8) | CHAR_AT(str, 11))), \
         ((uint16_t)((CHAR_AT(str, 12) << 8) | CHAR_AT(str, 13))), \
         ((uint16_t)((CHAR_AT(str, 14) << 8) | CHAR_AT(str, 15)))}
#else
    #define STR_TO_UINT16_ARRAY(str) \
        {((uint16_t)((CHAR_AT(str, 1) << 8) | CHAR_AT(str, 0))), \
         ((uint16_t)((CHAR_AT(str, 3) << 8) | CHAR_AT(str, 2))), \
         ((uint16_t)((CHAR_AT(str, 5) << 8) | CHAR_AT(str, 4))), \
         ((uint16_t)((CHAR_AT(str, 7) << 8) | CHAR_AT(str, 6))), \
         ((uint16_t)((CHAR_AT(str, 9) << 8) | CHAR_AT(str, 8))), \
         ((uint16_t)((CHAR_AT(str, 11) << 8) | CHAR_AT(str, 10))), \
         ((uint16_t)((CHAR_AT(str, 13) << 8) | CHAR_AT(str, 12))), \
         ((uint16_t)((CHAR_AT(str, 15) << 8) | CHAR_AT(str, 14)))}
#endif


/* Использвоние:
 * При прямом порядке байт
 * 
 * const uint16_t text[8] = STR_TO_UINT16_ARRAY(("HELLO123")); //4845 4C4C 4F31 3233 0000 0000 0000 0000 
 * 
 * При обратном - задефайнить SWAP_STRING 
 * const uint16_t text[8] = STR_TO_UINT16_ARRAY(("HELLO123"));// 4845 4C4C 4F31 3233 0000 0000 0000 0000 
 *
 * 
 * Но работает только с константами, инициализация не-константных значений не сработает.
 */

#endif //__UINT_STRINGS_H
