#if 0
#include <stdio.h>
#include <stdlib.h>
#endif

/* Each of these masks detects characters of zero to four bytes */
#define UTF8_TWO_BYTE_VAL   0x03
#define UTF8_THREE_BYTE_VAL 0X07
#define UTF8_FOUR_BYTE_VAL  0x0F

int utf8_get_len(unsigned char *str) {
    if(!(*str)) {
        return 0;
    } else if(!((*str) >> 7)) {
        return 1;
    } else if(((*str) >> 4) == UTF8_FOUR_BYTE_VAL) {
        /* The byte looks like: 11110xxx */
        printf("%02X%02X%02X%02X\n", *str, *(str + 1), *(str + 2), *(str + 3));
        return 4;
    } else if(((*str) >> 5) == UTF8_THREE_BYTE_VAL) {
        /* The byte looks like: 1110xxxx */
        printf("%02X%02X%02X\n", *str, *(str + 1), *(str + 2));
        return 3;
    } else if(((*str) >> 6) == UTF8_TWO_BYTE_VAL) {
        /* The byte looks like: 110xxxxx */
        printf("%02X%02X\n", *str, *(str + 1));
        return 2;
    } else {
        return -1;
    }
}

#if 0
int main() {
    char *line, *orig_line;
    size_t size, increment;
    int character_len;

    orig_line = NULL;
    line = NULL;
    size = 0;
    increment = 0;
    while(1) {
        if(increment == 0) {
            if(getline(&line, &size, stdin) == -1) {
                break;
            }
            orig_line = line;
        }
        character_len = utf8_get_len(line);
        if(character_len == 0) {
            increment = 0;
        } else if(character_len == 1) {
            printf("One-byte character.\n");
            increment = 1;
        } else if(character_len == 2) {
            printf("Two-byte character.\n");
            increment = 2;
        } else if(character_len == 3) {
            printf("Three-byte character.\n");
            increment = 3;
        } else if(character_len == 4) {
            printf("Four-byte character.\n");
            increment = 4;
        } else {
            printf("Character not recognized. Rest of string: '%s'\n", line);
            goto cleanup;
        }
        line += increment;
    }

cleanup:
    free(orig_line);
}
#endif
