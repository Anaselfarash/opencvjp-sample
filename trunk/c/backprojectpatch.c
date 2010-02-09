#include <cv.h>
#include <highgui.h>

int
main (int argc, char **argv)
{
  int i, hist_size = 90;
  float h_ranges[] = { 0, 180 };
  float *ranges[] = { h_ranges };
  double min_val, max_val;
  CvSize dst_size;
  CvPoint min_loc, max_loc;
  IplImage *src_img = 0, *tmp_img = 0, *dst_img;
  IplImage *src_hsv, *tmp_hsv;
  IplImage *src_planes[3], *tmp_planes[3];
  CvHistogram *hist = 0;
  char *imagename, *templatename;

  // (1)load a search-area image and a template image respectively
  imagename = argc > 1 ? argv[1] : "../image/room7.png";
  templatename = argc > 2 ? argv[2] : "../image/room7_temp.png";
  src_img = cvLoadImage (imagename, CV_LOAD_IMAGE_COLOR);
  tmp_img = cvLoadImage (templatename, CV_LOAD_IMAGE_COLOR);
  if(src_img == 0 || tmp_img == 0)
    return -1;

  // (2)create color planes buffer to calculate histograms
  for (i = 0; i < 3; i++) {
    src_planes[i] = cvCreateImage (cvGetSize (src_img), IPL_DEPTH_8U, 1);
    tmp_planes[i] = cvCreateImage (cvGetSize (tmp_img), IPL_DEPTH_8U, 1);
  }

  // (3)convert the color-space of input images from RGB to HSV
  src_hsv = cvCreateImage (cvGetSize (src_img), IPL_DEPTH_8U, 3);
  tmp_hsv = cvCreateImage (cvGetSize (tmp_img), IPL_DEPTH_8U, 3);
  cvCvtColor (src_img, src_hsv, CV_BGR2HSV);
  cvCvtColor (tmp_img, tmp_hsv, CV_BGR2HSV);
  cvCvtPixToPlane (src_hsv, src_planes[0], src_planes[1], src_planes[2], 0);
  cvCvtPixToPlane (tmp_hsv, tmp_planes[0], tmp_planes[1], tmp_planes[2], 0);

  // (4)calculate a histogram of hue-plane in a template image
  hist = cvCreateHist (1, &hist_size, CV_HIST_ARRAY, ranges, 1);
  cvCalcHist (&tmp_planes[0], hist, 0, 0);

  // (5)calculate distance between a template and a patch region in the source image
  //    by comparing their histograms
  dst_size = cvSize (src_img->width - tmp_img->width + 1,
		     src_img->height - tmp_img->height + 1);
  dst_img = cvCreateImage (dst_size, IPL_DEPTH_32F, 1);
  cvCalcBackProjectPatch (&src_planes[0], dst_img, cvGetSize (tmp_img), hist,
			  CV_COMP_CORREL, 1.0);
  cvMinMaxLoc (dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);

  // (6)draw a rectangle indicating template position and show the image
  cvRectangle (src_img, max_loc,
	       cvPoint (max_loc.x + tmp_img->width,
			max_loc.y + tmp_img->height), CV_RGB (255, 0, 0), 3, 8, 0);
  
  cvNamedWindow ("src", CV_WINDOW_AUTOSIZE);
  cvNamedWindow ("dst", CV_WINDOW_AUTOSIZE);
  cvShowImage ("src", src_img);
  cvShowImage ("dst", dst_img);
  cvWaitKey (0);
  
  cvDestroyWindow("Image");
  cvReleaseImage(&src_img);
  cvReleaseImage(&dst_img);
  cvReleaseImage(&tmp_img);
  cvReleaseImage(&src_hsv);
  cvReleaseImage(&tmp_hsv);
  for(i=0; i<3; i++) {
    cvReleaseImage(&src_planes[i]);
    cvReleaseImage(&tmp_planes[i]);
  }

  return 0;
}