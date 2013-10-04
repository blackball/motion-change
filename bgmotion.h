/**
 * detect if there some motion in the scene if a common 
 * operation. Here is a simple lib to acheive that goal.
 * 
 * the algorithm is very simple: just down sampling the image
 * do a thresh-ed differencing, then a simple median filtering,
 * and stat the white pixels.
 *
 * @blackball
 */

#include <stdlib.h>
#include <malloc.h>

#ifdef __cplusplus
extern "C" {
#endif 

static int BGMOTION_SAMPLE_STEP = 1;  /* skip ever 1 pixel at H/V directions */
static int BGMOTION_DIFF_THRESH = 10; /* if the diff > 10 then FG, else BG */
static int BGMOTION_MEDIAN_SIZE = 3;  /* radius of median filtering */

static struct bgmotion_t {
        int mw, mh, mws;
        int mp_num; /* motion pixels number */
        unsigned char *map;
        unsigned char *buf; 
};

static struct bgmotion_t* 
bgmotion_new(const unsigned char *data, int w, int ws, int h) {
        int i, j, mw, mh, mws;
        unsigned char *mdata = NULL;
        
        struct bgmotion_t *bgm = malloc(sizeof(*bgm));

        mw = (w + 1) / (1 + BGMOTION_SAMPLE_STEP);
        mh = (h + 1) / (1 + BGMOTION_SAMPLE_STEP);
        /* align to 4 byte */
        mws = (mw + 3) & ~(3);

        mdata = malloc(sizeof(*mdata) * mws * mh);

        bgm->map = mdata;

        for (i = 0; i < mh; ++i) {
                for (j = 0; j < mw; ++j) {
                        *mdata = *data;
                        
                        mdata++;
                        data += (1 + BGMOTION_SAMPLE_STEP);
                }
                mdata += mws - mw; /* skip the align gap */
                data += (ws - w) + ws * (BGMOTION_SAMPLE_STEP);
        }

        bgm->mw = mw;
        bgm->mh = mh;
        bgm->mws = mws;
        bgm->mp_num = 0;

        /* allocate the buffer chunk */
        bgm->buf = malloc(sizeof(bgm->buf[0]) * mws * mh);
        
        return bgm;
}

static void
bgmotion_free(struct bgmotion_t **bgm) {
        if (bgm && (*bgm)) {
                free( (*bgm)->map );
                free( (*bgm)->buf );
                free( *bgm );
                *bgm = NULL;
        }
}

static unsigned char
_bgmotion_median(const unsigned char *data, int w, int ws, int h, int cx, int cy, int size) {
        int x, y;
        int counter = 0;
        for (y = cy - size; y < cy + size; ++y) {
                for (x = cx - size; x < cx + size; ++x) {
                        unsigned char t = data[y * ws + x];
                        if (t == 255) {
                                counter++;
                        }
                        else {
                                counter--;
                        }
                }
        }
        
        return (counter > 0 ? 255 : 0);
}

/* first perfrom a simple thresh-ed subraction, then 
 * a median filtering on the result, finally we could 
 * count the motion pixels.
 */
static int
bgmotion_update(struct bgmotion_t *bgm, const unsigned char *data, int w, int ws, int h) {
        unsigned char *map = bgm->map;
        unsigned char *buf = bgm->buf;
        const int mw = bgm->mw, mws = bgm->mws, mh = bgm->mh;
        
        int i, j, count = 0;
        
        for (i = 0; i < mh; ++i) {
                for (j = 0; j < mw; ++j) {
                        *buf = (*map - *data) > BGMOTION_DIFF_THRESH ? 255 : 0;
                        
                        /* copy */
                        *map = *data;
                        
                        buf++;
                        map++;
                        data += (1 + BGMOTION_SAMPLE_STEP);
                }
                buf += mws - mw; /* skip the align gap */
                map += mws - mw;
                data += (ws - w) + ws * (BGMOTION_SAMPLE_STEP);
        }

        for (i = BGMOTION_MEDIAN_SIZE; i < mh - BGMOTION_MEDIAN_SIZE; ++i) {
                for (j = BGMOTION_MEDIAN_SIZE; j < mw - BGMOTION_MEDIAN_SIZE; ++j) {
                        unsigned int mv = _bgmotion_median(bgm->buf, mw, mws, mh, j, i, BGMOTION_MEDIAN_SIZE);
                        if (mv == 255) {
                                count ++;
                        }
                }                
        }
        
        bgm->mp_num = count;
        return 0;
}

static float 
bgmotion_ratio(const struct bgmotion_t *bgm) {
        return bgm->mp_num / (float)((bgm->mw - 2 * BGMOTION_MEDIAN_SIZE) * (bgm->mh - 2 * BGMOTION_MEDIAN_SIZE));
}

#ifdef __cplusplus
}
#endif 

