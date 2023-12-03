#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "pigun.h"
#include "pigun-mmal.h"
#include "pigun-detector.h"
#include "pigun-hid.h"

#include <math.h>


#include <X11/Xlib.h>
#include <X11/Xutil.h>




void pigun_detector_init(){

    pigun.detector.checked = calloc(PIGUN_RES_X * PIGUN_RES_Y, sizeof(uint8_t));
    pigun.detector.pxbuffer = (uint32_t*)malloc(sizeof(uint32_t) * PIGUN_NPX);
    
    pigun.detector.peaks = (pigun_peak_t*)calloc(10, sizeof(pigun_peak_t));
    
    pigun.detector.error = 0;
}

void pigun_detector_free(){

    free(pigun.detector.checked);
    free(pigun.detector.pxbuffer);
    free(pigun.detector.peaks);

}



/**
 * Performs a breadth-first search starting from the given starting index and
 * working on the given data array. Returns a list of indices found to belong
 * to the blob surrounding the starting point.
 * 
 * WARNING: if the blob is too big, it will be cutoff and its position will not be correct!
 * 
 * return 0 if the blob was too small
 * return 1 if the blob was ok
 */
int blob_detect(uint32_t idx, unsigned char* data, const uint32_t blobID, const float threshold) {
    
    uint32_t blobSize = 0;
    uint32_t x, y;
    uint64_t sumVal = 0;
    uint64_t sumX = 0, sumY = 0;
    uint32_t maxI = 0;

    // put the first px in the queue
    uint32_t qSize = 1; // length of the queue of idx to check
    pigun.detector.pxbuffer[0] = idx;
    pigun.detector.checked[idx] = 1;

#ifdef PIGUN_DEBUG
    printf("PIGUN: detecting peak...");
#endif

    // Do search until stack is emptied or maximum size is reached
    while (qSize > 0 && blobSize < DETECTOR_MAXBLOBSIZE) {
        
        // check the last element on the list
        qSize--;
        uint32_t current = pigun.detector.pxbuffer[qSize];
        
        // do the blob position computation
        
        // Transform flattened index to 2D coordinate
        x = current % PIGUN_RES_X;
        y = current / PIGUN_RES_X;
        sumVal += data[current];
        sumX += (uint64_t)(data[current] * x);
        sumY += (uint64_t)(data[current] * y);
        if (data[current] > maxI) maxI = data[current];
        
        blobSize++;

        // check neighbours
        uint32_t other;
        
        if(y > 0) { // UP
            other = current - PIGUN_RES_X;
            if (!pigun.detector.checked[other] && data[other] >= threshold) {
                pigun.detector.pxbuffer[qSize] = other;
                qSize++;
                pigun.detector.checked[other] = 1;
            }
        }
        if(y < PIGUN_RES_Y-1) { // DOWN
            other = current + PIGUN_RES_X;
            if (!pigun.detector.checked[other] && data[other] >= threshold) {
                pigun.detector.pxbuffer[qSize] = other;
                qSize++;
                pigun.detector.checked[other] = 1;
            }
        }
        if(x > 0) { // LEFT
            other = current - 1;
            if (!pigun.detector.checked[other] && data[other] >= threshold) {
                pigun.detector.pxbuffer[qSize] = other;
                qSize++;
                pigun.detector.checked[other] = 1;
            }
        }
        if(x < PIGUN_RES_X-1) { // RIGHT
            other = current + 1;
            if (!pigun.detector.checked[other] && data[other] >= threshold) {
                pigun.detector.pxbuffer[qSize] = other;
                qSize++;
                pigun.detector.checked[other] = 1;
            }
        }
    }
    // loop ends when there are no more px to check, or the blob is as big as it can be
    if (blobSize < DETECTOR_MINBLOBSIZE) return 0;

    // code here => peak was good, save it
    
    //printf("peak found[%i]: %li %li -- %li -- %i --> ", blobID, sumX, sumY, sumVal, blobSize);

    pigun.detector.peaks[blobID].col = ((float)sumX) / sumVal;
    pigun.detector.peaks[blobID].row = ((float)sumY) / sumVal;
    pigun.detector.peaks[blobID].maxI = (float)maxI;
    pigun.detector.peaks[blobID].total = (pigun.detector.peaks[blobID].row * PIGUN_RES_X + pigun.detector.peaks[blobID].col);
    
#ifdef PIGUN_DEBUG
    printf("%f %f\n", pigun.detector.peaks[blobID].col, pigun.detector.peaks[blobID].row);
#endif
    return 1;
}

int peak_compare(const void* a, const void* b) {

    pigun_peak_t* A = (pigun_peak_t*)a;
    pigun_peak_t* B = (pigun_peak_t*)b;
	// DESCENDING ORDER NOW!
    if (B->total > A->total) return -1;
    else return 1;
}




/**
    * Detects peaks in the camera output and reports them under the global
    * "peaks"-variables.
    */
void pigun_detector_run(unsigned char* data) {

    /*FILE* fout = fopen("text.bin", "rb");
    fread(data, sizeof(unsigned char), PIGUN_NPX, fout);
    fclose(fout);*/

#ifdef PIGUN_DEBUG
    printf("detecting...\n");
#endif

    // These parameters have to be tuned to optimize the search
    const float threshold = 130;          // The minimum threshold for pixel intensity in a blob

    const uint32_t nx = floor((float)(PIGUN_RES_X) / (float)(DETECTOR_DX));
    const uint32_t ny = floor((float)(PIGUN_RES_Y) / (float)(DETECTOR_DX));

    // Reset the boolean array for marking pixels as checked.
    memset(pigun.detector.checked, 0, PIGUN_RES_X * PIGUN_RES_Y * sizeof(uint8_t));


    // Here the order actually matters: we loop in this order to get better cache hit rate
    uint32_t blobID = 0;
    for (uint32_t j = 0; j < ny; ++j) {
        for (uint32_t i = 0; i < nx; ++i) {

            // pixel index in the buffer
            uint32_t idx = j * DETECTOR_DX * PIGUN_RES_X + i * DETECTOR_DX;
            uint8_t value = data[idx];

            // check if px is bright enough and not seen by the bfs before
            if (value >= threshold && !pigun.detector.checked[idx]) {
                
                // we found a bright pixel! search nearby
                value = blob_detect(idx, data, blobID, threshold);
                // peak was saved if good, move on to the next
                if (value == 1) {
                    blobID++;
                    // stop trying if we found the ones we deserve
                    if (blobID == DETECTOR_NBLOBS) break;
                }
            }
        }
        if (blobID == DETECTOR_NBLOBS) break;
    }

#ifdef PIGUN_DEBUG
    //printf("detector done, nblobs=%i/%i\n", blobID, DETECTOR_NBLOBS);
#endif

    // at this point we should have all the blobs we wanted
    // or maybe we are short
    if (blobID != DETECTOR_NBLOBS) {
        // if we are short or too many, tell the callback we got an error
        pigun.detector.error = 1;
        return;
    }

    /* INFO
        4 LED MODE:
	
        Order peaks. The ordering is based on the distance of the peaks to the screen corners:
            Peak closest to top-left corner = A
            Peak closest to bottom-left corner = B
            Peak closest to top-right corner = C
            Peak closest to bottom-right corner = D

        assuming we sort the peaks using the peak.total indexer (ascending), the camera sees:
	
        2---3      3---2
        |   |  OR  |   |
        0---1      1---0
	
        OR any similar pattern... in the end all we know is that:
        a. the first 2 peaks are the bottom LED bar
        b. the last 2 are the top LED bar
        but after sorting the ordering of top and bottom spots depends on
        camera rotation.
        we have to manually adjust them so that we get:

        0---1
        |   |
        2---3

        the aimer will use these in the correct order to compute the inverse projection!
    */

	// this will sort the peaks in descending peak.total order
	qsort(pigun.detector.peaks, 4, sizeof(pigun.detector.peaks), peak_compare);
	
	pigun_peak_t tmp;

	// now make sure 0 is the top-left of the first two peaks
	if(pigun.detector.peaks[0].col > pigun.detector.peaks[1].col){
		tmp = pigun.detector.peaks[0];
		pigun.detector.peaks[0] = pigun.detector.peaks[1];
		pigun.detector.peaks[1] = tmp;
	}

	// and that 2 is the bottom-left of the last two peaks
	if(pigun.detector.peaks[2].col > pigun.detector.peaks[3].col){
		tmp = pigun.detector.peaks[2];
		pigun.detector.peaks[2] = pigun.detector.peaks[3];
		pigun.detector.peaks[3] = tmp;
	}

    //printf("detector done [%i]\n",blobID);
    pigun.detector.error = 0;
    return;
}

