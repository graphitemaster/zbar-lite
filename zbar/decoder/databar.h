#ifndef _DATABAR_H_
#define _DATABAR_H_

#define DATABAR_MAX_SEGMENTS 32

/* active DataBar (partial) segment entry */
typedef struct databar_segment_s {
    signed finder : 5;          /* finder pattern */
    unsigned exp : 1;           /* DataBar expanded finder */
    unsigned color : 1;         /* finder coloring */
    unsigned side : 1;          /* data character side of finder */

    unsigned partial : 1;       /* unpaired partial segment */
    unsigned count : 7;         /* times encountered */
    unsigned epoch : 8;         /* age, in characters scanned */
    unsigned check : 8;         /* bar checksum */
    signed short data;          /* decoded character data */
    unsigned short width;       /* measured width of finder (14 modules) */
} databar_segment_t;

/* DataBar specific decode state */
typedef struct databar_decoder_s {
    unsigned config;            /* decoder configuration flags */
    unsigned config_exp;

    unsigned csegs : 8;         /* allocated segments */
    unsigned epoch : 8;         /* current scan */

    databar_segment_t *segs;    /* active segment list */
    signed char chars[16];      /* outstanding character indices */
} databar_decoder_t;

/* reset DataBar segment decode state */
static inline void databar_new_scan (databar_decoder_t *db) {
    int i;
    for(i = 0; i < 16; i++)
        if(db->chars[i] >= 0) {
            databar_segment_t *seg = db->segs + db->chars[i];
            if(seg->partial)
                seg->finder = -1;
            db->chars[i] = -1;
        }
}

/* reset DataBar accumulated segments */
static inline void databar_reset (databar_decoder_t *db) {
    int i, n = db->csegs;
    databar_new_scan(db);
    for(i = 0; i < n; i++)
        db->segs[i].finder = -1;
}

/* decode DataBar symbols */
zbar_symbol_type_t _zbar_decode_databar(zbar_decoder_t *dcode);

#endif
