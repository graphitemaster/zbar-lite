#ifndef _CODE93_H_
#define _CODE93_H_

/* Code 93 specific decode state */
typedef struct code93_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 3;       /* element offset 0-5 */
    int character : 12;         /* character position in symbol */
    unsigned width;             /* last character width */
    unsigned char buf;          /* first character */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} code93_decoder_t;

/* reset Code 93 specific state */
static inline void code93_reset (code93_decoder_t *dcode93) {
    dcode93->direction = 0;
    dcode93->element = 0;
    dcode93->character = -1;
}

/* decode Code 93 symbols */
zbar_symbol_type_t _zbar_decode_code93(zbar_decoder_t *dcode);

#endif
