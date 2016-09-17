#ifndef _CODABAR_H_
#define _CODABAR_H_

/* Codabar specific decode state */
typedef struct codabar_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd, 1=rev */
    unsigned element : 4;       /* element offset 0-7 */
    int character : 12;         /* character position in symbol */
    unsigned s7;                /* current character width */
    unsigned width;             /* last character width */
    unsigned char buf[6];       /* initial scan buffer */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} codabar_decoder_t;

/* reset Codabar specific state */
static inline void codabar_reset (codabar_decoder_t *codabar) {
    codabar->direction = 0;
    codabar->element = 0;
    codabar->character = -1;
    codabar->s7 = 0;
}

/* decode Codabar symbols */
zbar_symbol_type_t _zbar_decode_codabar(zbar_decoder_t *dcode);

#endif
