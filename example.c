#include "motion.h"
#include <opencv\cv.h>
#include <opencv\highgui.h>

int 
main(int argc, char *argv[]) {
	IplImage *frame = 0, *gray = 0;
	CvCapture *capture = 0;
	struct bgmotion_t *bgm = 0;
	
	capture = cvCaptureFromCAM(0);

	if (capture == NULL) {
		goto gt_yameidie;
	}

	frame = cvQueryFrame(capture);

	gray = cvCreateImage(cvGetSize(frame), 8, 1);

	cvCvtColor(frame, gray, CV_BGR2GRAY);

	bgm = bgmotion_new((const unsigned char*)gray->imageData, gray->width, gray->widthStep, gray->height);

	if (bgm == NULL) {
		goto gt_yameidie;
	}

	cvNamedWindow("camera", 1);
	while ( frame = cvQueryFrame(capture) ) {
		cvCvtColor(frame, gray, CV_BGR2GRAY);

		bgmotion_update(bgm, (const unsigned char *)gray->imageData, gray->width, gray->widthStep, gray->height);
		
		printf("%f\n", bgmotion_ratio(bgm));

		cvShowImage("camera", frame);
		if (cvWaitKey(10) == 27 ) {
			break;
		}

	}

        cvDestroyWindow("camera");
        
 gt_yameidie:
	if (capture) {
		cvReleaseCapture( &capture );
	}

	if (gray) {
		cvReleaseImage(&gray);
	}

	if (bgm) {
		bgmotion_free(&bgm);
	}

	return 0;
}
