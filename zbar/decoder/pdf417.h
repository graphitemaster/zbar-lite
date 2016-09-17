#ifndef _PDF417_H_
#define _PDF417_H_

/* PDF417 specific decode state */
typedef struct pdf417_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 3;       /* element offset 0-7 */
    int character : 12;         /* character position in symbol */
    unsigned s8;                /* character width */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} pdf417_decoder_t;

/* reset PDF417 specific state */
static inline void pdf417_reset (pdf417_decoder_t *pdf417) {
    pdf417->direction = 0;
    pdf417->element = 0;
    pdf417->character = -1;
    pdf417->s8 = 0;
}

/* decode PDF417 symbols */
zbar_symbol_type_t _zbar_decode_pdf417(zbar_decoder_t *dcode);

#endif
