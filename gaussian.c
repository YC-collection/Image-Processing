#include "gaussian.h"

void *thread_blur(void *arg)
{
    tInfo *info = arg;
    // working range : 2 ~ w-2 ; 2 ~ h-2 (size : w-4 , h-4)
    // calculate work load per thread (direction : col)
    int workload = (info->height - 4) / info->total_thread_size;
    for(int j=2+(workload)*(int)info->thread_id ; j < 2 + (workload)*((int)(info->thread_id)+1); j++) {
        for(int i=2; i < info->width-2 ; i++) {
            // do the image blur
            int sum = 0;
            sum = (int)global_src[(j-2)*info->width + i-2]*gaussian55[0] + global_src[(j-2)*info->width + i-1]*gaussian55[1]
                  + global_src[(j-2)*info->width + i]*gaussian55[2] + global_src[(j-2)*info->width + i+1]*gaussian55[3]
                  + global_src[(j-2)*info->width + i+2]*gaussian55[4] + global_src[(j-1)*info->width + i-2]*gaussian55[5]
                  + global_src[(j-1)*info->width + i-1]*gaussian55[6] + global_src[(j-1)*info->width + i]*gaussian55[7]
                  + global_src[(j-1)*info->width + i+1]*gaussian55[8] + global_src[(j-1)*info->width + i+2]*gaussian55[9]
                  + global_src[(j)*info->width + i-2]*gaussian55[10] + global_src[(j)*info->width + i-1]*gaussian55[11]
                  + global_src[(j)*info->width + i]*gaussian55[12] + global_src[(j)*info->width + i+1]*gaussian55[13]
                  + global_src[(j)*info->width + i+2]*gaussian55[14] + global_src[(j+1)*info->width + i-2]*gaussian55[15]
                  + global_src[(j+1)*info->width + i-1]*gaussian55[16] + global_src[(j+1)*info->width + i]*gaussian55[17]
                  + global_src[(j+1)*info->width + i+1]*gaussian55[18] + global_src[(j+1)*info->width + i+2]*gaussian55[19]
                  + global_src[(j+2)*info->width + i-2]*gaussian55[20] + global_src[(j+2)*info->width + i-1]*gaussian55[21]
                  + global_src[(j+2)*info->width + i]*gaussian55[22] + global_src[(j+2)*info->width + i+1]*gaussian55[23]
                  + global_src[(j+2)*info->width + i+2]*gaussian55[24];
            global_src[j*info->width + i] = ((sum / 273) > 255 ? 255 : sum/273);
        }
    }
    return NULL;
}

void *sse_thread_blur(void *arg)
{
    tInfo *info = arg;
    const __m128i vk0 = _mm_set1_epi8(0);
    const unsigned char sse_g1_lo[16] = {1,0,1,0,1,0,4,0,4,0,4,0,7,0,7,0};
    const unsigned char sse_g1_hi[16] = {7,0,4,0,4,0,4,0,1,0,1,0,1,0,0,0};
    const unsigned char sse_g2_lo[16] = {4,0,4,0,4,0,16,0,16,0,16,0,26,0,26,0};
    const unsigned char sse_g2_hi[16] = {26,0,16,0,16,0,16,0,4,0,4,0,4,0,0,0};
    const unsigned char sse_g3_lo[16] = {7,0,7,0,7,0,26,0,26,0,26,0,41,0,41,0};
    const unsigned char sse_g3_hi[16] = {41,0,26,0,26,0,26,0,7,0,7,0,7,0,0,0};
    const unsigned char sse_g4_lo[16] = {4,0,4,0,4,0,16,0,16,0,16,0,26,0,26,0};
    const unsigned char sse_g4_hi[16] = {26,0,16,0,16,0,16,0,4,0,4,0,4,0,0,0};
    const unsigned char sse_g5_lo[16] = {1,0,1,0,1,0,4,0,4,0,4,0,7,0,7,0};
    const unsigned char sse_g5_hi[16] = {7,0,4,0,4,0,4,0,1,0,1,0,1,0,0,0};
    int workload = (info->height - 4) / info->total_thread_size;
    for(int i=0; i < info->width-5 ; i++) {
        for(int j=(workload)*(int)info->thread_id ; j < (workload)*((int)(info->thread_id)+1)-5 ; j++) {
            int sum_r = 0,sum_g = 0,sum_b = 0;
            __m128i vg1lo = _mm_loadu_si128((__m128i *)sse_g1_lo);
            __m128i vg1hi = _mm_loadu_si128((__m128i *)sse_g1_hi);
            __m128i vg2lo = _mm_loadu_si128((__m128i *)sse_g2_lo);
            __m128i vg2hi = _mm_loadu_si128((__m128i *)sse_g2_hi);
            __m128i vg3lo = _mm_loadu_si128((__m128i *)sse_g3_lo);
            __m128i vg3hi = _mm_loadu_si128((__m128i *)sse_g3_hi);
            __m128i vg4lo = _mm_loadu_si128((__m128i *)sse_g4_lo);
            __m128i vg4hi = _mm_loadu_si128((__m128i *)sse_g4_hi);
            __m128i vg5lo = _mm_loadu_si128((__m128i *)sse_g5_lo);
            __m128i vg5hi = _mm_loadu_si128((__m128i *)sse_g5_hi);

            __m128i L0 = _mm_loadu_si128((__m128i *)(global_src_ori+(j+0)*info->width + i));
            __m128i L1 = _mm_loadu_si128((__m128i *)(global_src_ori+(j+1)*info->width + i));
            __m128i L2 = _mm_loadu_si128((__m128i *)(global_src_ori+(j+2)*info->width + i));
            __m128i L3 = _mm_loadu_si128((__m128i *)(global_src_ori+(j+3)*info->width + i));
            __m128i L4 = _mm_loadu_si128((__m128i *)(global_src_ori+(j+4)*info->width + i));

            __m128i v0lo = _mm_unpacklo_epi8(L0,vk0);
            __m128i v0hi = _mm_unpackhi_epi8(L0,vk0);
            __m128i v1lo = _mm_unpacklo_epi8(L1,vk0);
            __m128i v1hi = _mm_unpackhi_epi8(L1,vk0);
            __m128i v2lo = _mm_unpacklo_epi8(L2,vk0);
            __m128i v2hi = _mm_unpackhi_epi8(L2,vk0);
            __m128i v3lo = _mm_unpacklo_epi8(L3,vk0);
            __m128i v3hi = _mm_unpackhi_epi8(L3,vk0);
            __m128i v4lo = _mm_unpacklo_epi8(L4,vk0);
            __m128i v4hi = _mm_unpackhi_epi8(L4,vk0);

            v0lo = _mm_maddubs_epi16(v0lo,vg1lo);
            v0hi = _mm_maddubs_epi16(v0hi,vg1hi);
            v1lo = _mm_maddubs_epi16(v1lo,vg2lo);
            v1hi = _mm_maddubs_epi16(v1hi,vg2hi);
            v2lo = _mm_maddubs_epi16(v2lo,vg3lo);
            v2hi = _mm_maddubs_epi16(v2hi,vg3hi);
            v3lo = _mm_maddubs_epi16(v3lo,vg4lo);
            v3hi = _mm_maddubs_epi16(v3hi,vg4hi);
            v4lo = _mm_maddubs_epi16(v4lo,vg5lo);
            v4hi = _mm_maddubs_epi16(v4hi,vg5hi);

            __m128i vsumlo = _mm_set1_epi16(0),vsumhi = _mm_set1_epi16(0),vtemp_1 = _mm_set1_epi16(0),vtemp_2 = _mm_set1_epi16(0),vtemp_3 = _mm_set1_epi16(0),vtemp_4 = _mm_set1_epi16(0);
            vsumlo = _mm_add_epi16(vsumlo,v0lo);
            vsumhi = _mm_add_epi16(vsumhi,v0hi);
            vsumlo = _mm_add_epi16(vsumlo,v1lo);
            vsumhi = _mm_add_epi16(vsumhi,v1hi);
            vsumlo = _mm_add_epi16(vsumlo,v2lo);
            vsumhi = _mm_add_epi16(vsumhi,v2hi);
            vsumlo = _mm_add_epi16(vsumlo,v3lo);
            vsumhi = _mm_add_epi16(vsumhi,v3hi);
            vsumlo = _mm_add_epi16(vsumlo,v4lo);
            vsumhi = _mm_add_epi16(vsumhi,v4hi);

            // Get each R,G,B
            vtemp_1 = _mm_unpacklo_epi16(vsumlo,vk0);
            vtemp_2 = _mm_unpackhi_epi16(vsumlo,vk0);
            vtemp_3 = _mm_unpacklo_epi16(vsumhi,vk0);
            vtemp_4 = _mm_unpackhi_epi16(vsumhi,vk0);

            // Get sum
            sum_b += _mm_cvtsi128_si32(vtemp_1) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,4)) + _mm_cvtsi128_si32(vtemp_4);
            sum_g += _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,4)) + _mm_cvtsi128_si32(vtemp_2) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_4,4));
            sum_r += _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,4)) + _mm_cvtsi128_si32(vtemp_3) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_4,8));

            global_src_ori[j*info->width+i].rgbRed = ((sum_r/273) > 255 ) ? 255 : sum_r/273 ;
            global_src_ori[j*info->width+i].rgbGreen = ((sum_g/273) > 255 ) ? 255 : sum_g/273 ;
            global_src_ori[j*info->width+i].rgbBlue = ((sum_b/273) > 255 ) ? 255 : sum_b/273 ;
        }
    }
    return NULL;
}

void pt_gaussian_blur_5_tri(unsigned char *src,int num_threads,int w,int h)
{
    // calculate for width (For a pthread to go through)
    pthread_t *thread_handler;
    tInfo *threadInfo;
    global_src = src;
    /* Allocate memory for pthread_create() arguments */
    thread_handler = malloc(num_threads*sizeof(pthread_t));
    threadInfo = malloc(num_threads*sizeof(tInfo));
    /* Create one thread for each command-line argument */
    for(int tnum = 0; tnum < num_threads ; tnum++) {
        //tInfo *threadInfo = malloc(sizeof(tInfo));
        threadInfo[tnum].thread_id = tnum;
        threadInfo[tnum].total_thread_size = num_threads;
        threadInfo[tnum].width = w;
        threadInfo[tnum].height = h;
        pthread_create(&thread_handler[tnum],NULL,thread_blur,&threadInfo[tnum]);
    }
    /* Now join with each thread */
    for(int tnum = 0; tnum < num_threads ; tnum++) {
        pthread_join(thread_handler[tnum],NULL);
    }

    global_src = NULL;
    free(threadInfo);
    free(thread_handler);
}

void pt_sse_gaussian_blur_5_ori(RGBTRIPLE *src,int num_threads,int w,int h)
{
    pthread_t *thread_handler;
    tInfo *threadInfo;
    global_src_ori = src;
    thread_handler = malloc(num_threads*sizeof(pthread_t));
    threadInfo = malloc(num_threads*sizeof(tInfo));

    /* Create one thread for each command-line argument */
    for(int tnum = 0; tnum < num_threads ; tnum++) {
        threadInfo[tnum].thread_id = tnum;
        threadInfo[tnum].total_thread_size = num_threads;
        threadInfo[tnum].width = w;
        threadInfo[tnum].height = h;
        pthread_create(&thread_handler[tnum],NULL,sse_thread_blur,&threadInfo[tnum]);
    }
    for(int tnum = 0; tnum < num_threads ; tnum++) {
        pthread_join(thread_handler[tnum],NULL);
    }

    global_src_ori = NULL;
    free(thread_handler);
    free(threadInfo);
}

void unroll_gaussian_blur_5_tri(unsigned char *src,int w,int h)
{
    for(int j=2; j<h-2; j++) {
        for(int i=2; i<w-2; i++) {
            int sum = 0;
            // Unroll the 5x5 for loop
            sum = (int)src[(j-2)*w+(i-2)]*gaussian55[0] + src[(j-2)*w+(i-1)]*gaussian55[1]
                  + src[(j-2)*w+(i)]*gaussian55[2] + src[(j-2)*w+(i+1)]*gaussian55[3]
                  + src[(j-2)*w+(i+2)]*gaussian55[4] + src[(j-1)*w+(i-2)]*gaussian55[5]
                  + src[(j-1)*w+(i-1)]*gaussian55[6] + src[(j-1)*w+(i)]*gaussian55[7]
                  + src[(j-1)*w+(i+1)]*gaussian55[8] + src[(j-1)*w+(i+2)]*gaussian55[9]
                  + src[(j)*w+(i-2)]*gaussian55[10] + src[(j)*w+(i-1)]*gaussian55[11]
                  + src[(j)*w+(i)]*gaussian55[12] + src[(j)*w+(i+1)]*gaussian55[13]
                  + src[(j)*w+(i+2)]*gaussian55[14] + src[(j+1)*w+(i-2)]*gaussian55[15]
                  + src[(j+1)*w+(i-1)]*gaussian55[16] + src[(j+1)*w+(i)]*gaussian55[17]
                  + src[(j+1)*w+(i+1)]*gaussian55[18] + src[(j+1)*w+(i+2)]*gaussian55[19]
                  + src[(j+2)*w+(i-2)]*gaussian55[20] + src[(j+2)*w+(i-1)]*gaussian55[21]
                  + src[(j+2)*w+(i)]*gaussian55[22] + src[(j+2)*w+(i+1)]*gaussian55[23]
                  + src[(j+2)*w+(i+2)]*gaussian55[24];

            src[j*w+i] = ((sum / 273) > 255 ? 255 : sum/273);
        }
    }
}

void unroll_gaussian_blur_5_ori(RGBTRIPLE *src,int w,int h)
{
    for(int j=2; j<h-2; j++) {
        for(int i=2; i<w-2; i++) {
            int sum_r = 0,sum_g = 0,sum_b = 0;
            // Unroll the 5x5 for loop
            sum_r = (int)src[(j-2)*w+(i-2)].rgbRed*gaussian55[0] + src[(j-2)*w+(i-1)].rgbRed*gaussian55[1]
                    + src[(j-2)*w+(i)].rgbRed*gaussian55[2] + src[(j-2)*w+(i+1)].rgbRed*gaussian55[3]
                    + src[(j-2)*w+(i+2)].rgbRed*gaussian55[4] + src[(j-1)*w+(i-2)].rgbRed*gaussian55[5]
                    + src[(j-1)*w+(i-1)].rgbRed*gaussian55[6] + src[(j-1)*w+(i)].rgbRed*gaussian55[7]
                    + src[(j-1)*w+(i+1)].rgbRed*gaussian55[8] + src[(j-1)*w+(i+2)].rgbRed*gaussian55[9]
                    + src[(j)*w+(i-2)].rgbRed*gaussian55[10] + src[(j)*w+(i-1)].rgbRed*gaussian55[11]
                    + src[(j)*w+(i)].rgbRed*gaussian55[12] + src[(j)*w+(i+1)].rgbRed*gaussian55[13]
                    + src[(j)*w+(i+2)].rgbRed*gaussian55[14] + src[(j+1)*w+(i-2)].rgbRed*gaussian55[15]
                    + src[(j+1)*w+(i-1)].rgbRed*gaussian55[16] + src[(j+1)*w+(i)].rgbRed*gaussian55[17]
                    + src[(j+1)*w+(i+1)].rgbRed*gaussian55[18] + src[(j+1)*w+(i+2)].rgbRed*gaussian55[19]
                    + src[(j+2)*w+(i-2)].rgbRed*gaussian55[20] + src[(j+2)*w+(i-1)].rgbRed*gaussian55[21]
                    + src[(j+2)*w+(i)].rgbRed*gaussian55[22] + src[(j+2)*w+(i+1)].rgbRed*gaussian55[23]
                    + src[(j+2)*w+(i+2)].rgbRed*gaussian55[24];
            sum_g = (int)src[(j-2)*w+(i-2)].rgbGreen*gaussian55[0] + src[(j-2)*w+(i-1)].rgbGreen*gaussian55[1]
                    + src[(j-2)*w+(i)].rgbGreen*gaussian55[2] + src[(j-2)*w+(i+1)].rgbGreen*gaussian55[3]
                    + src[(j-2)*w+(i+2)].rgbGreen*gaussian55[4] + src[(j-1)*w+(i-2)].rgbGreen*gaussian55[5]
                    + src[(j-1)*w+(i-1)].rgbGreen*gaussian55[6] + src[(j-1)*w+(i)].rgbGreen*gaussian55[7]
                    + src[(j-1)*w+(i+1)].rgbGreen*gaussian55[8] + src[(j-1)*w+(i+2)].rgbGreen*gaussian55[9]
                    + src[(j)*w+(i-2)].rgbGreen*gaussian55[10] + src[(j)*w+(i-1)].rgbGreen*gaussian55[11]
                    + src[(j)*w+(i)].rgbGreen*gaussian55[12] + src[(j)*w+(i+1)].rgbGreen*gaussian55[13]
                    + src[(j)*w+(i+2)].rgbGreen*gaussian55[14] + src[(j+1)*w+(i-2)].rgbGreen*gaussian55[15]
                    + src[(j+1)*w+(i-1)].rgbGreen*gaussian55[16] + src[(j+1)*w+(i)].rgbGreen*gaussian55[17]
                    + src[(j+1)*w+(i+1)].rgbGreen*gaussian55[18] + src[(j+1)*w+(i+2)].rgbGreen*gaussian55[19]
                    + src[(j+2)*w+(i-2)].rgbGreen*gaussian55[20] + src[(j+2)*w+(i-1)].rgbGreen*gaussian55[21]
                    + src[(j+2)*w+(i)].rgbGreen*gaussian55[22] + src[(j+2)*w+(i+1)].rgbGreen*gaussian55[23]
                    + src[(j+2)*w+(i+2)].rgbGreen*gaussian55[24];
            sum_b = (int)src[(j-2)*w+(i-2)].rgbBlue*gaussian55[0] + src[(j-2)*w+(i-1)].rgbBlue*gaussian55[1]
                    + src[(j-2)*w+(i)].rgbBlue*gaussian55[2] + src[(j-2)*w+(i+1)].rgbBlue*gaussian55[3]
                    + src[(j-2)*w+(i+2)].rgbBlue*gaussian55[4] + src[(j-1)*w+(i-2)].rgbBlue*gaussian55[5]
                    + src[(j-1)*w+(i-1)].rgbBlue*gaussian55[6] + src[(j-1)*w+(i)].rgbBlue*gaussian55[7]
                    + src[(j-1)*w+(i+1)].rgbBlue*gaussian55[8] + src[(j-1)*w+(i+2)].rgbBlue*gaussian55[9]
                    + src[(j)*w+(i-2)].rgbBlue*gaussian55[10] + src[(j)*w+(i-1)].rgbBlue*gaussian55[11]
                    + src[(j)*w+(i)].rgbBlue*gaussian55[12] + src[(j)*w+(i+1)].rgbBlue*gaussian55[13]
                    + src[(j)*w+(i+2)].rgbBlue*gaussian55[14] + src[(j+1)*w+(i-2)].rgbBlue*gaussian55[15]
                    + src[(j+1)*w+(i-1)].rgbBlue*gaussian55[16] + src[(j+1)*w+(i)].rgbBlue*gaussian55[17]
                    + src[(j+1)*w+(i+1)].rgbBlue*gaussian55[18] + src[(j+1)*w+(i+2)].rgbBlue*gaussian55[19]
                    + src[(j+2)*w+(i-2)].rgbBlue*gaussian55[20] + src[(j+2)*w+(i-1)].rgbBlue*gaussian55[21]
                    + src[(j+2)*w+(i)].rgbBlue*gaussian55[22] + src[(j+2)*w+(i+1)].rgbBlue*gaussian55[23]
                    + src[(j+2)*w+(i+2)].rgbBlue*gaussian55[24];

            sum_r = ((sum_r / 273) > 255 ? 255 : sum_r/273);
            sum_g = ((sum_g / 273) > 255 ? 255 : sum_g/273);
            sum_b = ((sum_b / 273) > 255 ? 255 : sum_b/273);
            src[j*w+i].rgbRed = sum_r;
            src[j*w+i].rgbGreen = sum_g;
            src[j*w+i].rgbBlue = sum_b;
        }
    }
}

void naive_gaussian_blur_5(unsigned char *src,int w,int h)
{
    for(int j=2; j<h-2; j++) {
        for(int i=2; i<w-2; i++) {
            int sum = 0;
            int index = 0;
            for(int sqr_j=j-2; sqr_j<j+3; sqr_j++) {
                for(int sqr_i=i-2; sqr_i<i+3; sqr_i++) {
                    sum += (int)src[sqr_j*w+sqr_i]*gaussian55[index++];
                }
            }
            sum /= 273;
            if(sum > 255)
                sum = 255;
            src[j*w+i] = sum;
        }
    }
}

void naive_gaussian_blur_5_expand(unsigned char *src,int w,int h)
{
    uint32_t *out = malloc(w*h*sizeof(uint32_t));
    memset(out,0,w*h*sizeof(uint32_t));
    for(int j=2; j<h-2; j++) {
        for(int i=2; i<w-2; i++) {
            unsigned int temp = src[j*w+i];
            out[(j-2)*w+i-2] += temp*gaussian55[0];
            out[(j-2)*w+i-1] += temp*gaussian55[1];
            out[(j-2)*w+i] += temp*gaussian55[2];
            out[(j-2)*w+i+1] += temp*gaussian55[3];
            out[(j-2)*w+i+2] += temp*gaussian55[4];
            out[(j-1)*w+i-2] += temp*gaussian55[5];
            out[(j-1)*w+i-1] += temp*gaussian55[6];
            out[(j-1)*w+i] += temp*gaussian55[7];
            out[(j-1)*w+i+1] += temp*gaussian55[8];
            out[(j-1)*w+i+2] += temp*gaussian55[9];
            out[(j)*w+i-2] += temp*gaussian55[10];
            out[(j)*w+i-1] += temp*gaussian55[11];
            out[(j)*w+i] += temp*gaussian55[12];
            out[(j)*w+i+1] += temp*gaussian55[13];
            out[(j)*w+i+2] += temp*gaussian55[14];
            out[(j+1)*w+i-2] += temp*gaussian55[15];
            out[(j+1)*w+i-1] += temp*gaussian55[16];
            out[(j+1)*w+i] += temp*gaussian55[17];
            out[(j+1)*w+i+1] += temp*gaussian55[18];
            out[(j+1)*w+i+2] += temp*gaussian55[19];
            out[(j+2)*w+i-2] += temp*gaussian55[20];
            out[(j+2)*w+i-1] += temp*gaussian55[21];
            out[(j+2)*w+i] += temp*gaussian55[22];
            out[(j+2)*w+i+1] += temp*gaussian55[23];
            out[(j+2)*w+i+2] += temp*gaussian55[24];
        }
    }

    for(int j=0; j<h; j++) {
        for(int i=0; i<w; i++) {
            src[j*w+i] = ((out[j*w+i]/=deno55) > 255) ? 255: out[j*w+i];
        }
    }

    free(out);
}

void naive_gaussian_blur_5_original(RGBTRIPLE *src,int w,int h)
{
    for(int j=2; j<h-2; j++) {
        for(int i=2; i<w-2; i++) {
            int sum_r = 0,sum_g = 0, sum_b = 0;
            int index = 0;
            for(int sqr_j=j-2; sqr_j<j+3; sqr_j++) {
                for(int sqr_i=i-2; sqr_i<i+3; sqr_i++) {
                    sum_r += (int)src[sqr_j*w+sqr_i].rgbRed*gaussian55[index];
                    sum_g += (int)src[sqr_j*w+sqr_i].rgbGreen*gaussian55[index];
                    sum_b += (int)src[sqr_j*w+sqr_i].rgbBlue*gaussian55[index];
                    index++;
                }
            }
            src[j*w+i].rgbRed = ((sum_r/273) > 255 ) ? 255 : sum_r/273 ;
            src[j*w+i].rgbGreen = ((sum_g/273) > 255 ) ? 255 : sum_g/273 ;
            src[j*w+i].rgbBlue = ((sum_b/273) > 255 ) ? 255 : sum_b/273 ;
        }
    }
}

void sse_gaussian_blur_5_tri(unsigned char *src,int w,int h)
{
    // const data
    const __m128i vk0 = _mm_set1_epi8(0);
    const unsigned char sse_g1[16] = { 1,0,4,0,7,0,4,0,1,0,0,0,0,0,0,0 };
    const unsigned char sse_g2[16] = { 4,0,16,0,26,0,16,0,4,0,0,0,0,0,0,0 };
    const unsigned char sse_g3[16] = { 7,0,26,0,41,0,26,0,7,0,0,0,0,0,0,0 };
    const unsigned char sse_g4[16] = { 4,0,16,0,26,0,16,0,4,0,0,0,0,0,0,0 };
    const unsigned char sse_g5[16] = { 1,0,4,0,7,0,4,0,1,0,0,0,0,0,0,0 };
    // Operation to image
    for(int i=0; i<w-5; i++) {
        for(int j=0; j<h-5; j++) {
            int sum = 0;
            __m128i vg1 = _mm_loadu_si128((__m128i *)sse_g1);
            __m128i vg2 = _mm_loadu_si128((__m128i *)sse_g2);
            __m128i vg3 = _mm_loadu_si128((__m128i *)sse_g3);
            __m128i vg4 = _mm_loadu_si128((__m128i *)sse_g4);
            __m128i vg5 = _mm_loadu_si128((__m128i *)sse_g5);

            __m128i vsum = _mm_set1_epi8(0),vtemplow = _mm_set1_epi8(0),vtemphigh = _mm_set1_epi8(0),vempty = _mm_set1_epi8(0);
            // First element src[j*w+i]
            // Load in data
            __m128i L0 = _mm_loadu_si128((__m128i *)(src+(j+0)*w + i));
            __m128i L1 = _mm_loadu_si128((__m128i *)(src+(j+1)*w + i));
            __m128i L2 = _mm_loadu_si128((__m128i *)(src+(j+2)*w + i));
            __m128i L3 = _mm_loadu_si128((__m128i *)(src+(j+3)*w + i));
            __m128i L4 = _mm_loadu_si128((__m128i *)(src+(j+4)*w + i));
            // Get the data we need (5 element per-line) , because we only
            // need 5 element from sse instruction set , so only get low part(contain 8 elements)
            __m128i v0 = _mm_unpacklo_epi8(L0,vk0);
            __m128i v1 = _mm_unpacklo_epi8(L1,vk0);
            __m128i v2 = _mm_unpacklo_epi8(L2,vk0);
            __m128i v3 = _mm_unpacklo_epi8(L3,vk0);
            __m128i v4 = _mm_unpacklo_epi8(L4,vk0);
            // Multiple with specific Gaussian coef.
            v0 = _mm_maddubs_epi16(v0,vg1);
            v1 = _mm_maddubs_epi16(v1,vg2);
            v2 = _mm_maddubs_epi16(v2,vg3);
            v3 = _mm_maddubs_epi16(v3,vg4);
            v4 = _mm_maddubs_epi16(v4,vg5);
            // Summation the 5 line
            vsum = _mm_add_epi16(vsum,v0);
            vsum = _mm_add_epi16(vsum,v1);
            vsum = _mm_add_epi16(vsum,v2);
            vsum = _mm_add_epi16(vsum,v3);
            vsum = _mm_add_epi16(vsum,v4);
            // Vsum summation
            // Summation all - (Summation all - (Summation with shift-off 5 number))
            vtemplow = _mm_unpacklo_epi16(vsum,vempty); // 1,2,3,4
            vtemphigh = _mm_unpackhi_epi16(vsum,vempty); // 5

            sum += _mm_cvtsi128_si32(vtemplow); // get 1
            sum += _mm_cvtsi128_si32(_mm_srli_si128(vtemplow,4)); // get 2
            sum += _mm_cvtsi128_si32(_mm_srli_si128(vtemplow,8)); // get 3
            sum += _mm_cvtsi128_si32(_mm_srli_si128(vtemplow,12)); // get 4
            sum += _mm_cvtsi128_si32(vtemphigh); // get 5

            sum /= deno55;
            if(sum > 255)
                sum = 255;
            src[(j+2)*w+(i+2)] = sum;
        }
    }
}

void sse_gaussian_blur_5_ori(RGBTRIPLE *src,int w,int h)
{
    const __m128i vk0 = _mm_set1_epi8(0);
    const unsigned char sse_g1_lo[16] = {1,0,1,0,1,0,4,0,4,0,4,0,7,0,7,0};
    const unsigned char sse_g1_hi[16] = {7,0,4,0,4,0,4,0,1,0,1,0,1,0,0,0};
    const unsigned char sse_g2_lo[16] = {4,0,4,0,4,0,16,0,16,0,16,0,26,0,26,0};
    const unsigned char sse_g2_hi[16] = {26,0,16,0,16,0,16,0,4,0,4,0,4,0,0,0};
    const unsigned char sse_g3_lo[16] = {7,0,7,0,7,0,26,0,26,0,26,0,41,0,41,0};
    const unsigned char sse_g3_hi[16] = {41,0,26,0,26,0,26,0,7,0,7,0,7,0,0,0};
    const unsigned char sse_g4_lo[16] = {4,0,4,0,4,0,16,0,16,0,16,0,26,0,26,0};
    const unsigned char sse_g4_hi[16] = {26,0,16,0,16,0,16,0,4,0,4,0,4,0,0,0};
    const unsigned char sse_g5_lo[16] = {1,0,1,0,1,0,4,0,4,0,4,0,7,0,7,0};
    const unsigned char sse_g5_hi[16] = {7,0,4,0,4,0,4,0,1,0,1,0,1,0,0,0};
    for(int i=0; i<(w-5); i++) {
        for(int j=0; j<(h-5); j++) {
            int sum_r = 0,sum_g = 0,sum_b = 0;
            __m128i vg1lo = _mm_loadu_si128((__m128i *)sse_g1_lo);
            __m128i vg1hi = _mm_loadu_si128((__m128i *)sse_g1_hi);
            __m128i vg2lo = _mm_loadu_si128((__m128i *)sse_g2_lo);
            __m128i vg2hi = _mm_loadu_si128((__m128i *)sse_g2_hi);
            __m128i vg3lo = _mm_loadu_si128((__m128i *)sse_g3_lo);
            __m128i vg3hi = _mm_loadu_si128((__m128i *)sse_g3_hi);
            __m128i vg4lo = _mm_loadu_si128((__m128i *)sse_g4_lo);
            __m128i vg4hi = _mm_loadu_si128((__m128i *)sse_g4_hi);
            __m128i vg5lo = _mm_loadu_si128((__m128i *)sse_g5_lo);
            __m128i vg5hi = _mm_loadu_si128((__m128i *)sse_g5_hi);

            __m128i L0 = _mm_loadu_si128((__m128i *)(src+(j+0)*w + i));
            __m128i L1 = _mm_loadu_si128((__m128i *)(src+(j+1)*w + i));
            __m128i L2 = _mm_loadu_si128((__m128i *)(src+(j+2)*w + i));
            __m128i L3 = _mm_loadu_si128((__m128i *)(src+(j+3)*w + i));
            __m128i L4 = _mm_loadu_si128((__m128i *)(src+(j+4)*w + i));

            __m128i v0lo = _mm_unpacklo_epi8(L0,vk0);
            __m128i v0hi = _mm_unpackhi_epi8(L0,vk0);
            __m128i v1lo = _mm_unpacklo_epi8(L1,vk0);
            __m128i v1hi = _mm_unpackhi_epi8(L1,vk0);
            __m128i v2lo = _mm_unpacklo_epi8(L2,vk0);
            __m128i v2hi = _mm_unpackhi_epi8(L2,vk0);
            __m128i v3lo = _mm_unpacklo_epi8(L3,vk0);
            __m128i v3hi = _mm_unpackhi_epi8(L3,vk0);
            __m128i v4lo = _mm_unpacklo_epi8(L4,vk0);
            __m128i v4hi = _mm_unpackhi_epi8(L4,vk0);

            v0lo = _mm_maddubs_epi16(v0lo,vg1lo);
            v0hi = _mm_maddubs_epi16(v0hi,vg1hi);
            v1lo = _mm_maddubs_epi16(v1lo,vg2lo);
            v1hi = _mm_maddubs_epi16(v1hi,vg2hi);
            v2lo = _mm_maddubs_epi16(v2lo,vg3lo);
            v2hi = _mm_maddubs_epi16(v2hi,vg3hi);
            v3lo = _mm_maddubs_epi16(v3lo,vg4lo);
            v3hi = _mm_maddubs_epi16(v3hi,vg4hi);
            v4lo = _mm_maddubs_epi16(v4lo,vg5lo);
            v4hi = _mm_maddubs_epi16(v4hi,vg5hi);

            __m128i vsumlo = _mm_set1_epi16(0),vsumhi = _mm_set1_epi16(0),vtemp_1 = _mm_set1_epi16(0),vtemp_2 = _mm_set1_epi16(0),vtemp_3 = _mm_set1_epi16(0),vtemp_4 = _mm_set1_epi16(0);
            vsumlo = _mm_add_epi16(vsumlo,v0lo);
            vsumhi = _mm_add_epi16(vsumhi,v0hi);
            vsumlo = _mm_add_epi16(vsumlo,v1lo);
            vsumhi = _mm_add_epi16(vsumhi,v1hi);
            vsumlo = _mm_add_epi16(vsumlo,v2lo);
            vsumhi = _mm_add_epi16(vsumhi,v2hi);
            vsumlo = _mm_add_epi16(vsumlo,v3lo);
            vsumhi = _mm_add_epi16(vsumhi,v3hi);
            vsumlo = _mm_add_epi16(vsumlo,v4lo);
            vsumhi = _mm_add_epi16(vsumhi,v4hi);

            // Get each R,G,B
            vtemp_1 = _mm_unpacklo_epi16(vsumlo,vk0);
            vtemp_2 = _mm_unpackhi_epi16(vsumlo,vk0);
            vtemp_3 = _mm_unpacklo_epi16(vsumhi,vk0);
            vtemp_4 = _mm_unpackhi_epi16(vsumhi,vk0);

            // Get sum
            sum_b += _mm_cvtsi128_si32(vtemp_1) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,4)) + _mm_cvtsi128_si32(vtemp_4);
            sum_g += _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,4)) + _mm_cvtsi128_si32(vtemp_2) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_4,4));
            sum_r += _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,4)) + _mm_cvtsi128_si32(vtemp_3) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_4,8));

            src[j*w+i].rgbRed = ((sum_r/273) > 255 ) ? 255 : sum_r/273 ;
            src[j*w+i].rgbGreen = ((sum_g/273) > 255 ) ? 255 : sum_g/273 ;
            src[j*w+i].rgbBlue = ((sum_b/273) > 255 ) ? 255 : sum_b/273 ;
        }
    }
}

void sse_gaussian_blur_5_prefetch_ori(RGBTRIPLE *src,int w,int h)
{
    const __m128i vk0 = _mm_set1_epi8(0);
    const unsigned char sse_g1_lo[16] = {1,0,1,0,1,0,4,0,4,0,4,0,7,0,7,0};
    const unsigned char sse_g1_hi[16] = {7,0,4,0,4,0,4,0,1,0,1,0,1,0,0,0};
    const unsigned char sse_g2_lo[16] = {4,0,4,0,4,0,16,0,16,0,16,0,26,0,26,0};
    const unsigned char sse_g2_hi[16] = {26,0,16,0,16,0,16,0,4,0,4,0,4,0,0,0};
    const unsigned char sse_g3_lo[16] = {7,0,7,0,7,0,26,0,26,0,26,0,41,0,41,0};
    const unsigned char sse_g3_hi[16] = {41,0,26,0,26,0,26,0,7,0,7,0,7,0,0,0};
    const unsigned char sse_g4_lo[16] = {4,0,4,0,4,0,16,0,16,0,16,0,26,0,26,0};
    const unsigned char sse_g4_hi[16] = {26,0,16,0,16,0,16,0,4,0,4,0,4,0,0,0};
    const unsigned char sse_g5_lo[16] = {1,0,1,0,1,0,4,0,4,0,4,0,7,0,7,0};
    const unsigned char sse_g5_hi[16] = {7,0,4,0,4,0,4,0,1,0,1,0,1,0,0,0};
    for(int i=0; i<(w-5); i++) {
        for(int j=0; j<(h-5); j++) {
            _mm_prefetch(src+(j + 16 + 0) *w + i, _MM_HINT_T0);
            _mm_prefetch(src+(j + 16 + 1) *w + i, _MM_HINT_T0);
            _mm_prefetch(src+(j + 16 + 2) *w + i, _MM_HINT_T0);
            _mm_prefetch(src+(j + 16 + 3) *w + i, _MM_HINT_T0);
            _mm_prefetch(src+(j + 16 + 4) *w + i, _MM_HINT_T0);
            int sum_r = 0,sum_g = 0,sum_b = 0;
            __m128i vg1lo = _mm_loadu_si128((__m128i *)sse_g1_lo);
            __m128i vg1hi = _mm_loadu_si128((__m128i *)sse_g1_hi);
            __m128i vg2lo = _mm_loadu_si128((__m128i *)sse_g2_lo);
            __m128i vg2hi = _mm_loadu_si128((__m128i *)sse_g2_hi);
            __m128i vg3lo = _mm_loadu_si128((__m128i *)sse_g3_lo);
            __m128i vg3hi = _mm_loadu_si128((__m128i *)sse_g3_hi);
            __m128i vg4lo = _mm_loadu_si128((__m128i *)sse_g4_lo);
            __m128i vg4hi = _mm_loadu_si128((__m128i *)sse_g4_hi);
            __m128i vg5lo = _mm_loadu_si128((__m128i *)sse_g5_lo);
            __m128i vg5hi = _mm_loadu_si128((__m128i *)sse_g5_hi);

            __m128i L0 = _mm_loadu_si128((__m128i *)(src+(j+0)*w + i));
            __m128i L1 = _mm_loadu_si128((__m128i *)(src+(j+1)*w + i));
            __m128i L2 = _mm_loadu_si128((__m128i *)(src+(j+2)*w + i));
            __m128i L3 = _mm_loadu_si128((__m128i *)(src+(j+3)*w + i));
            __m128i L4 = _mm_loadu_si128((__m128i *)(src+(j+4)*w + i));

            __m128i v0lo = _mm_unpacklo_epi8(L0,vk0);
            __m128i v0hi = _mm_unpackhi_epi8(L0,vk0);
            __m128i v1lo = _mm_unpacklo_epi8(L1,vk0);
            __m128i v1hi = _mm_unpackhi_epi8(L1,vk0);
            __m128i v2lo = _mm_unpacklo_epi8(L2,vk0);
            __m128i v2hi = _mm_unpackhi_epi8(L2,vk0);
            __m128i v3lo = _mm_unpacklo_epi8(L3,vk0);
            __m128i v3hi = _mm_unpackhi_epi8(L3,vk0);
            __m128i v4lo = _mm_unpacklo_epi8(L4,vk0);
            __m128i v4hi = _mm_unpackhi_epi8(L4,vk0);

            v0lo = _mm_maddubs_epi16(v0lo,vg1lo);
            v0hi = _mm_maddubs_epi16(v0hi,vg1hi);
            v1lo = _mm_maddubs_epi16(v1lo,vg2lo);
            v1hi = _mm_maddubs_epi16(v1hi,vg2hi);
            v2lo = _mm_maddubs_epi16(v2lo,vg3lo);
            v2hi = _mm_maddubs_epi16(v2hi,vg3hi);
            v3lo = _mm_maddubs_epi16(v3lo,vg4lo);
            v3hi = _mm_maddubs_epi16(v3hi,vg4hi);
            v4lo = _mm_maddubs_epi16(v4lo,vg5lo);
            v4hi = _mm_maddubs_epi16(v4hi,vg5hi);

            __m128i vsumlo = _mm_set1_epi16(0),vsumhi = _mm_set1_epi16(0),vtemp_1 = _mm_set1_epi16(0),vtemp_2 = _mm_set1_epi16(0),vtemp_3 = _mm_set1_epi16(0),vtemp_4 = _mm_set1_epi16(0);
            vsumlo = _mm_add_epi16(vsumlo,v0lo);
            vsumhi = _mm_add_epi16(vsumhi,v0hi);
            vsumlo = _mm_add_epi16(vsumlo,v1lo);
            vsumhi = _mm_add_epi16(vsumhi,v1hi);
            vsumlo = _mm_add_epi16(vsumlo,v2lo);
            vsumhi = _mm_add_epi16(vsumhi,v2hi);
            vsumlo = _mm_add_epi16(vsumlo,v3lo);
            vsumhi = _mm_add_epi16(vsumhi,v3hi);
            vsumlo = _mm_add_epi16(vsumlo,v4lo);
            vsumhi = _mm_add_epi16(vsumhi,v4hi);

            // Get each R,G,B
            vtemp_1 = _mm_unpacklo_epi16(vsumlo,vk0);
            vtemp_2 = _mm_unpackhi_epi16(vsumlo,vk0);
            vtemp_3 = _mm_unpacklo_epi16(vsumhi,vk0);
            vtemp_4 = _mm_unpackhi_epi16(vsumhi,vk0);

            // Get sum
            sum_b += _mm_cvtsi128_si32(vtemp_1) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,4)) + _mm_cvtsi128_si32(vtemp_4);
            sum_g += _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,4)) + _mm_cvtsi128_si32(vtemp_2) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_4,4));
            sum_r += _mm_cvtsi128_si32(_mm_srli_si128(vtemp_1,8)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_2,4)) + _mm_cvtsi128_si32(vtemp_3) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_3,12)) + _mm_cvtsi128_si32(_mm_srli_si128(vtemp_4,8));

            src[j*w+i].rgbRed = ((sum_r/273) > 255 ) ? 255 : sum_r/273 ;
            src[j*w+i].rgbGreen = ((sum_g/273) > 255 ) ? 255 : sum_g/273 ;
            src[j*w+i].rgbBlue = ((sum_b/273) > 255 ) ? 255 : sum_b/273 ;
        }
    }
}

void unroll_gaussian_1D_tri(RGBTRIPLE *src,int w,int h)
{
    for(int j=0; j<h; j++) {
        for(int i=2; i<w-2; i++) {
            float sum_r = (float)src[j*w+i-2].rgbRed*gaussian15[0] + src[j*w+i-1].rgbRed*gaussian15[1]
                          + src[j*w+i].rgbRed*gaussian15[2] + src[j*w+i+1].rgbRed*gaussian15[3]
                          + src[j*w+i+2].rgbRed*gaussian15[4];
            float sum_g = (float)src[j*w+i-2].rgbGreen*gaussian15[0] + src[j*w+i-1].rgbGreen*gaussian15[1]
                          + src[j*w+i].rgbGreen*gaussian15[2] + src[j*w+i+1].rgbGreen*gaussian15[3]
                          + src[j*w+i+2].rgbGreen*gaussian15[4];
            float sum_b = (float)src[j*w+i-2].rgbBlue*gaussian15[0] + src[j*w+i-1].rgbBlue*gaussian15[1]
                          + src[j*w+i].rgbBlue*gaussian15[2] + src[j*w+i+1].rgbBlue*gaussian15[3]
                          + src[j*w+i+2].rgbBlue*gaussian15[4];
            src[j*w+i].rgbRed = (sum_r > 255 ) ? 255 : sum_r;
            src[j*w+i].rgbGreen = (sum_g > 255 ) ? 255 : sum_g;
            src[j*w+i].rgbBlue = (sum_b > 255 ) ? 255 : sum_b;
        }
    }
    for(int j=2; j<h-2; j++) {
        for(int i=0; i<w; i++) {
            float sum_r = (float)src[(j-2)*w+i].rgbRed*gaussian15[0] + src[(j-1)*w+i].rgbRed*gaussian15[1]
                          + src[j*w+i].rgbRed*gaussian15[2] + src[(j+1)*w+i].rgbRed*gaussian15[3]
                          + src[(j+2)*w+i].rgbRed*gaussian15[4];
            float sum_g = (float)src[(j-2)*w+i].rgbGreen*gaussian15[0] + src[(j-1)*w+i].rgbGreen*gaussian15[1]
                          + src[j*w+i].rgbGreen*gaussian15[2] + src[(j+1)*w+i].rgbGreen*gaussian15[3]
                          + src[(j+2)*w+i].rgbGreen*gaussian15[4];
            float sum_b = (float)src[(j-2)*w+i].rgbBlue*gaussian15[0] + src[(j-1)*w+i].rgbBlue*gaussian15[1]
                          + src[j*w+i].rgbBlue*gaussian15[2] + src[(j+1)*w+i].rgbBlue*gaussian15[3]
                          + src[(j+2)*w+i].rgbBlue*gaussian15[4];
            src[j*w+i].rgbRed = (sum_r > 255 ) ? 255 : sum_r;
            src[j*w+i].rgbGreen = (sum_g > 255 ) ? 255 : sum_g;
            src[j*w+i].rgbBlue = (sum_b > 255 ) ? 255 : sum_b;
        }
    }
}
