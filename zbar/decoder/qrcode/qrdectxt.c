/*Copyright (C) 2008-2010  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qrcode.h"
#include "qrdec.h"
#include "util.h"
#include "image.h"
#include "decoder.h"
#include "img_scanner.h"

int qr_code_data_list_extract_text(const qr_code_data_list *_qrlist,
                                   zbar_image_scanner_t *iscn,
                                   zbar_image_t *img) {
    const qr_code_data  *qrdata;
    int                  nqrdata;
    unsigned char       *mark;
    int                  ntext;
    int                  i;
    qrdata=_qrlist->qrdata;
    nqrdata=_qrlist->nqrdata;
    mark=(unsigned char *)calloc(nqrdata,sizeof(*mark));
    ntext=0;
    for(i=0; i<nqrdata; i++)if(!mark[i]) {
            const qr_code_data       *qrdataj;
            const qr_code_data_entry *entry;
            int                       sa[16];
            int                       sa_size;
            char                     *sa_text;
            size_t                    sa_ntext;
            size_t                    sa_ctext;
            int                       fnc1;
            int                       fnc1_2ai;
            int                       err;
            int                       j;
            int                       k;
            zbar_symbol_t *syms = NULL, **sym = &syms;
            qr_point dir;
            int horiz;

            /*Step 0: Collect the other QR codes belonging to this S-A group.*/
            if(qrdata[i].sa_size) {
                unsigned sa_parity;
                sa_size=qrdata[i].sa_size;
                sa_parity=qrdata[i].sa_parity;
                for(j=0; j<sa_size; j++)sa[j]=-1;
                for(j=i; j<nqrdata; j++)if(!mark[j]) {
                        /*TODO: We could also match version, ECC level, etc. if size and
                           parity alone are too ambiguous.*/
                        if(qrdata[j].sa_size==sa_size&&qrdata[j].sa_parity==sa_parity&&
                                sa[qrdata[j].sa_index]<0) {
                            sa[qrdata[j].sa_index]=j;
                            mark[j]=1;
                        }
                    }
                /*TODO: If the S-A group is complete, check the parity.*/
            } else {
                sa[0]=i;
                sa_size=1;
            }

            sa_ctext=0;
            fnc1=0;
            fnc1_2ai=0;
            /*Step 1: Detect FNC1 markers and estimate the required buffer size.*/
            for(j=0; j<sa_size; j++)if(sa[j]>=0) {
                    qrdataj=qrdata+sa[j];
                    for(k=0; k<qrdataj->nentries; k++) {
                        int shift;
                        entry=qrdataj->entries+k;
                        shift=0;
                        switch(entry->mode) {
                        /*FNC1 applies to the entire code and ignores subsequent markers.*/
                        case QR_MODE_FNC1_1ST: {
                            if(!fnc1)fnc1=MOD(ZBAR_MOD_GS1);
                        }
                        break;
                        case QR_MODE_FNC1_2ND: {
                            if(!fnc1) {
                                fnc1=MOD(ZBAR_MOD_AIM);
                                fnc1_2ai=entry->payload.ai;
                                sa_ctext+=2;
                            }
                        }
                        break;
                        case QR_MODE_BYTE:
                            shift=2;
                        default: {
                            /*The remaining two modes are already valid UTF-8.*/
                            if(QR_MODE_HAS_DATA(entry->mode)) {
                                sa_ctext+=entry->payload.data.len<<shift;
                            }
                        }
                        break;
                        }
                    }
                }

            /*Step 2: Convert the entries.*/
            sa_text=(char *)malloc((sa_ctext+1)*sizeof(*sa_text));
            sa_ntext=0;
            /*Add the encoded Application Indicator for FNC1 in the second position.*/
            if(fnc1==MOD(ZBAR_MOD_AIM)) {
                if(fnc1_2ai<100) {
                    /*The Application Indicator is a 2-digit number.*/
                    sa_text[sa_ntext++]='0'+fnc1_2ai/10;
                    sa_text[sa_ntext++]='0'+fnc1_2ai%10;
                }
                /*The Application Indicator is a single letter.
                  We already checked that it lies in one of the ranges A...Z, a...z
                   when we decoded it.*/
                else sa_text[sa_ntext++]=(char)(fnc1_2ai-100);
            }

            err=0;
            for(j = 0; j < sa_size && !err; j++, sym = &(*sym)->next) {
                *sym = _zbar_image_scanner_alloc_sym(iscn, ZBAR_QRCODE, 0);
                (*sym)->datalen = sa_ntext;
                if(sa[j]<0) {
                    /* generic placeholder for unfinished results */
                    (*sym)->type = ZBAR_PARTIAL;

                    /*Skip all contiguous missing segments.*/
                    for(j++; j<sa_size&&sa[j]<0; j++);
                    /*If there aren't any more, stop.*/
                    if(j>=sa_size)break;

                    /* mark break in data */
                    sa_text[sa_ntext++]='\0';
                    (*sym)->datalen = sa_ntext;

                    /* advance to next symbol */
                    sym = &(*sym)->next;
                    *sym = _zbar_image_scanner_alloc_sym(iscn, ZBAR_QRCODE, 0);
                }

                qrdataj=qrdata+sa[j];
                /* expose bounding box */
                sym_add_point(*sym, qrdataj->bbox[0][0], qrdataj->bbox[0][1]);
                sym_add_point(*sym, qrdataj->bbox[2][0], qrdataj->bbox[2][1]);
                sym_add_point(*sym, qrdataj->bbox[3][0], qrdataj->bbox[3][1]);
                sym_add_point(*sym, qrdataj->bbox[1][0], qrdataj->bbox[1][1]);

                /* approx symbol "up" direction */
                dir[0] = (qrdataj->bbox[0][0] - qrdataj->bbox[2][0] +
                          qrdataj->bbox[1][0] - qrdataj->bbox[3][0]);
                dir[1] = (qrdataj->bbox[2][1] - qrdataj->bbox[0][1] +
                          qrdataj->bbox[3][1] - qrdataj->bbox[1][1]);
                horiz = abs(dir[0]) > abs(dir[1]);
                (*sym)->orient = horiz + 2 * (dir[1 - horiz] < 0);

                for(k=0; k<qrdataj->nentries&&!err; k++) {
                    size_t              inleft;
                    char               *in;
                    entry=qrdataj->entries+k;
                    switch(entry->mode) {
                    case QR_MODE_NUM: {
                        if(sa_ctext-sa_ntext>=(size_t)entry->payload.data.len) {
                            memcpy(sa_text+sa_ntext,entry->payload.data.buf,
                                   entry->payload.data.len*sizeof(*sa_text));
                            sa_ntext+=entry->payload.data.len;
                        } else err=1;
                    }
                    break;
                    case QR_MODE_BYTE:
                    case QR_MODE_ALNUM: {
                        char *p;
                        in=(char *)entry->payload.data.buf;
                        inleft=entry->payload.data.len;
                        /*FNC1 uses '%' as an escape character.*/
                        if(fnc1)for(;;) {
                                size_t plen;
                                char   c;
                                p=memchr(in,'%',inleft*sizeof(*in));
                                if(p==NULL)break;
                                plen=p-in;
                                if(sa_ctext-sa_ntext<plen+1)break;
                                memcpy(sa_text+sa_ntext,in,plen*sizeof(*in));
                                sa_ntext+=plen;
                                /*Two '%'s is a literal '%'*/
                                if(plen+1<inleft&&p[1]=='%') {
                                    c='%';
                                    plen++;
                                    p++;
                                }
                                /*One '%' is the ASCII group separator.*/
                                else c=0x1D;
                                sa_text[sa_ntext++]=c;
                                inleft-=plen+1;
                                in=p+1;
                            }
                        else p=NULL;
                        if(p!=NULL||sa_ctext-sa_ntext<inleft)err=1;
                        else {
                            memcpy(sa_text+sa_ntext,in,inleft*sizeof(*sa_text));
                            sa_ntext+=inleft;
                        }
                    }
                    break;
                    /*Silence stupid compiler warnings.*/
                    default:
                        break;
                    }
                }
            }
            if(!err) {
                zbar_symbol_t *sa_sym;
                sa_text[sa_ntext++]='\0';
                if(sa_ctext+1>sa_ntext) {
                    sa_text=(char *)realloc(sa_text,sa_ntext*sizeof(*sa_text));
                }

                if(sa_size == 1)
                    sa_sym = syms;
                else {
                    /* cheap out w/axis aligned bbox for now */
                    int xmin = img->width, xmax = -2;
                    int ymin = img->height, ymax = -2;

                    /* create "virtual" container symbol for composite result */
                    sa_sym = _zbar_image_scanner_alloc_sym(iscn, ZBAR_QRCODE, 0);
                    sa_sym->syms = _zbar_symbol_set_create();
                    sa_sym->syms->head = syms;

                    /* fixup data references */
                    for(; syms; syms = syms->next) {
                        int next;
                        _zbar_symbol_refcnt(syms, 1);
                        if(syms->type == ZBAR_PARTIAL)
                            sa_sym->type = ZBAR_PARTIAL;
                        else
                            for(j = 0; j < (int)syms->npts; j++) {
                                int u = syms->pts[j].x;
                                if(xmin >= u) xmin = u - 1;
                                if(xmax <= u) xmax = u + 1;
                                u = syms->pts[j].y;
                                if(ymin >= u) ymin = u - 1;
                                if(ymax <= u) ymax = u + 1;
                            }
                        syms->data = sa_text + syms->datalen;
                        next = (syms->next) ? syms->next->datalen : sa_ntext;
                        assert(next > (int)syms->datalen);
                        syms->datalen = next - syms->datalen - 1;
                    }
                    if(xmax >= -1) {
                        sym_add_point(sa_sym, xmin, ymin);
                        sym_add_point(sa_sym, xmin, ymax);
                        sym_add_point(sa_sym, xmax, ymax);
                        sym_add_point(sa_sym, xmax, ymin);
                    }
                }
                sa_sym->data = sa_text;
                sa_sym->data_alloc = sa_ntext;
                sa_sym->datalen = sa_ntext - 1;
                sa_sym->modifiers = fnc1;

                _zbar_image_scanner_add_sym(iscn, sa_sym);
            } else {
                _zbar_image_scanner_recycle_syms(iscn, syms);
                free(sa_text);
            }
        }
    free(mark);
    return ntext;
}
