
#include <inttypes.h>
#include <unistd.h>

#ifndef PIGUN_DETECTOR
#define PIGUN_DETECTOR


#define DETECTOR_DX 4               // number of skipped pixels in the coarse search
#define DETECTOR_MINBLOBSIZE 20     // minimum number of bright px that can be considered a blob
#define DETECTOR_MAXBLOBSIZE 1000   // maximum numer of pixels for a blob
#define DETECTOR_NBLOBS 4           // number of blobs that the detector will look for


typedef struct {

    uint8_t *checked;   // one element for each px in the image
    uint32_t *pxbuffer; // this is used by blob_detect to store the px indexes in the queue - the total allocation is PIGUN_RES_X* PIGUN_RES_Y


    
}pigun_detector_t;




void pigun_detector_init();
void pigun_detector_run(unsigned char*);


#endif
