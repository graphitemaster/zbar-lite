#ifndef _I25_H_
#define _I25_H_

/* interleaved 2 of 5 specific decode state */
typedef struct i25_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 4;       /* element offset 0-8 */
    int character : 12;         /* character position in symbol */
    unsigned s10;               /* current character width */
    unsigned width;             /* last character width */
    unsigned char buf[4];       /* initial scan buffer */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} i25_decoder_t;

/* reset interleaved 2 of 5 specific state */
static inline void i25_reset (i25_decoder_t *i25) {
    i25->direction = 0;
    i25->element = 0;
    i25->character = -1;
    i25->s10 = 0;
}

/* decode interleaved 2 of 5 symbols */
zbar_symbol_type_t _zbar_decode_i25(zbar_decoder_t *dcode);

#endif
