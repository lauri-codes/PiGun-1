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
#include "pigun-hid.h"
#include <math.h>


#include <X11/Xlib.h>
#include <X11/Xutil.h>

// How many pixels are skipped by the coarse detector
#define DETECTOR_DX 4

//vector<bool> CHECKED(PIGUN_RES_X* PIGUN_RES_Y, false);			// Boolean array for storing which pixel locations have been checked in the blob detection
unsigned char* checked;
unsigned int* pxbuffer; // this is used by blob_detect to store the px indexes in the queue - the total allocation is PIGUN_RES_X* PIGUN_RES_Y

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
int blob_detect(uint32_t idx, unsigned char* data, const uint32_t blobID, const float threshold, const uint32_t minBlobSize, const uint32_tt maxBlobSize) {
    
    uint32_t blobSize = 0;
    uint32_t x, y;
    uint64_t sumVal = 0;
    uint64_t sumX = 0, sumY = 0;
    uint32_t maxI = 0;

    // put the first px in the queue
    unsigned int qSize = 1; // length of the queue of idx to check
    pxbuffer[0] = idx;
    checked[idx] = 1;

#ifdef PIGUN_DEBUG
    printf("PIGUN: detecting peak...");
#endif

    // Do search until stack is emptied or maximum size is reached
    while (qSize > 0 && blobSize < maxBlobSize) {
        
        // check the last element on the list
        qSize--;
        int current = pxbuffer[qSize];
        
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
        int other;
        other = current - PIGUN_RES_X;
        if (other >= 0 && !checked[other] && data[other] >= threshold && y > 0) {
            pxbuffer[qSize] = other;
            qSize++;
            checked[other] = 1;
        }

        other = current + PIGUN_RES_X;
        if (other >= 0 && !checked[other] && data[other] >= threshold && y < PIGUN_RES_Y-1) {
            pxbuffer[qSize] = other;
            qSize++;
            checked[other] = 1;
        }

        other = current - 1;
        if (other >= 0 && !checked[other] && data[other] >= threshold && x > 0) {
            pxbuffer[qSize] = other;
            qSize++;
            checked[other] = 1;
        }

        other = current + 1;
        if (other >= 0 && !checked[other] && data[other] >= threshold && x < PIGUN_RES_X-1) {
            pxbuffer[qSize] = other;
            qSize++;
            checked[other] = 1;
        }
    }
    // loop ends when there are no more px to check, or the blob is as big as it can be
    if (blobSize < minBlobSize) return 0;

    // code here => peak was good, save it
    
    //printf("peak found[%i]: %li %li -- %li -- %i --> ", blobID, sumX, sumY, sumVal, blobSize);

    pigun.peaks[blobID].col = ((float)sumX) / sumVal;
    pigun.peaks[blobID].row = ((float)sumY) / sumVal;
    pigun.peaks[blobID].maxI = (float)maxI;
    pigun.peaks[blobID].total = (pigun.peaks[blobID].row * PIGUN_RES_X + pigun.peaks[blobID].col);
    
#ifdef PIGUN_DEBUG
    printf("%f %f\n", pigun.peaks[blobID].col, pigun.peaks[blobID].row);
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
int pigun_detect(unsigned char* data) {

    /*FILE* fout = fopen("text.bin", "rb");
    fread(data, sizeof(unsigned char), PIGUN_NPX, fout);
    fclose(fout);*/

#ifdef PIGUN_DEBUG
    printf("detecting...\n");
#endif

    // These parameters have to be tuned to optimize the search
    // How many blobs to search
    const unsigned int nBlobs = 4;
    const unsigned int dx = 4;            // How many pixels are skipped in x direction
    const unsigned int dy = 4;            // How many pixels are skipped in y direction
    const unsigned int minBlobSize = 20;  // Have many pixels does a blob have to have to be considered valid
    const unsigned int maxBlobSize =1000; // Maximum numer of pixels for a blob
    const float threshold = 130;          // The minimum threshold for pixel intensity in a blob

    const unsigned int nx = floor((float)(PIGUN_RES_X) / (float)(dx));
    const unsigned int ny = floor((float)(PIGUN_RES_Y) / (float)(dy));

    // Reset the boolean array for marking pixels as checked.
    if (checked == NULL) {
        checked = calloc(PIGUN_RES_X * PIGUN_RES_Y, sizeof(unsigned char));
        pxbuffer = malloc(sizeof(unsigned int) * PIGUN_RES_X * PIGUN_RES_Y);
    }
    memset(checked, 0, PIGUN_RES_X * PIGUN_RES_Y * sizeof(unsigned char));


    // Here the order actually matters: we loop in this order to get better cache
    // hit rate
    unsigned int blobID = 0;
    for(uint32_t idx=0; idx<PIGUN_NPX; idx+=DETECTOR_DX){
    //for (unsigned int j = 0; j < ny; ++j) {
        //for (unsigned int i = 0; i < nx; ++i) {

            // pixel index in the buffer
            //int idx = j * dy * PIGUN_RES_X + i * dx;
            uint8_t value = data[idx];

            // check if px is bright enough and not seen by the bfs before
            if (value >= threshold && !checked[idx]) {
                
                // we found a bright pixel! search nearby
                value = blob_detect(idx, data, blobID, threshold, minBlobSize, maxBlobSize);
                // peak was saved if good, move on to the next
                if (value == 1) {
                    blobID++;
                    // stop trying if we found the ones we deserve
                    if (blobID == nBlobs) break;
                }
            }
        }
        if (blobID == nBlobs) break;
    }

#ifdef PIGUN_DEBUG
    //printf("detector done, nblobs=%i/%i\n", blobID, nBlobs);
#endif

    // at this point we should have all the blobs we wanted
    // or maybe we are short
    if (blobID != nBlobs) {
        // if we are short, tell the callback we got an error
        return 1;
    }
    
    // Order peaks. The ordering is based on the distance of the peaks to
    // the screen corners:
    // Peak closest to top-left corner = A
    // Peak closest to bottom-left corner = B
    // Peak closest to top-right corner = C
    // Peak closest to bottom-right corner = D

    /* 
        4 LED MODE:
	
        assuming we sort the peaks using the peak.total indexer (ascending),
        the camera sees:
	
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
	qsort(pigun.peaks, 4, sizeof(pigun.peaks), peak_compare);
	
	pigun_peak_t tmp;

	// now make sure 0 is the top-left of the first two peaks
	if(pigun.peaks[0].col > pigun.peaks[1].col){
		tmp = pigun.peaks[0];
		pigun.peaks[0] = pigun.peaks[1];
		pigun.peaks[1] = tmp;
	}

	// and that 2 is the bottom-left of the last two peaks
	if(pigun.peaks[2].col > pigun.peaks[3].col){
		tmp = pigun.peaks[2];
		pigun.peaks[2] = pigun.peaks[3];
		pigun.peaks[3] = tmp;
	}

    //printf("detector done [%i]\n",blobID);
    return 0;
}

