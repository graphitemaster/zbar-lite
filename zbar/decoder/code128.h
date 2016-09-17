#ifndef _CODE128_H_
#define _CODE128_H_

/* Code 128 specific decode state */
typedef struct code128_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 3;       /* element offset 0-5 */
    int character : 12;         /* character position in symbol */
    unsigned char start;        /* start character */
    unsigned s6;                /* character width */
    unsigned width;             /* last character width */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} code128_decoder_t;

/* reset Code 128 specific state */
static inline void code128_reset (code128_decoder_t *dcode128) {
    dcode128->direction = 0;
    dcode128->element = 0;
    dcode128->character = -1;
    dcode128->s6 = 0;
}

/* decode Code 128 symbols */
zbar_symbol_type_t _zbar_decode_code128(zbar_decoder_t *dcode);

#endif
