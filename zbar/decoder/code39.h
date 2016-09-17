
#ifndef _CODE39_H_
#define _CODE39_H_

/* Code 39 specific decode state */
typedef struct code39_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd, 1=rev */
    unsigned element : 4;       /* element offset 0-8 */
    int character : 12;         /* character position in symbol */
    unsigned s9;                /* current character width */
    unsigned width;             /* last character width */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} code39_decoder_t;

/* reset Code 39 specific state */
static inline void code39_reset (code39_decoder_t *dcode39) {
    dcode39->direction = 0;
    dcode39->element = 0;
    dcode39->character = -1;
    dcode39->s9 = 0;
}

/* decode Code 39 symbols */
zbar_symbol_type_t _zbar_decode_code39(zbar_decoder_t *dcode);

#endif
