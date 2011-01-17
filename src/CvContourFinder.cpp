
#include "CvContourFinder.h"
#include "CinderOpenCv.h"

namespace cinder{

	//--------------------------------------------------------------------------------
	bool sort_carea_compare( const vector<cv::Point>& a, const vector<cv::Point>& b) {
		// use opencv to calc size, then sort based on size
		float areaa = fabs(cv::contourArea(cv::Mat(a)));
		float areab = fabs(cv::contourArea(cv::Mat(b)));

		//return 0;
		return (areaa > areab);
	}




	//--------------------------------------------------------------------------------
	CvContourFinder::CvContourFinder() {
		_width = 0;
		_height = 0;
		myMoments = cv::Moments();
		reset();
	}

	//--------------------------------------------------------------------------------
	CvContourFinder::~CvContourFinder() {
		
	}

	//--------------------------------------------------------------------------------
	void CvContourFinder::reset() {
		contoursVec.clear();
		blobs.clear();
		nBlobs = 0;
	}

	//--------------------------------------------------------------------------------
	int CvContourFinder::findContours( Surface&  input,
										  int minArea,
										  int maxArea,
										  int nConsidered,
										  bool bFindHoles,
										  bool bUseApproximation) {

		// get width/height disregarding ROI

		reset();

		// opencv will clober the image it detects contours on, so we want to
		// copy it into a copy before we detect contours.  That copy is allocated
		// if necessary (necessary = (a) not allocated or (b) wrong size)
		// so be careful if you pass in different sized images to "findContours"
		// there is a performance penalty, but we think there is not a memory leak
		// to worry about better to create mutiple contour finders for different
		// sizes, ie, if you are finding contours in a 640x480 image but also a
		// 320x240 image better to make two ofxCvContourFinder objects then to use
		// one, because you will get penalized less.

	//	inputCopy = Surface( _width, _height );

	 //   inputCopy = input;
	 //   inputCopy.setROI( input.getROI() );
		
		cv::Mat ipmat (toOcv(input));
		cv::Mat gray;
		cv::cvtColor( ipmat, gray, CV_RGB2GRAY );
		cv::findContours(gray, contoursVec, (bFindHoles) ? CV_RETR_LIST : CV_RETR_EXTERNAL, bUseApproximation ? CV_CHAIN_APPROX_SIMPLE : CV_CHAIN_APPROX_NONE );

		// put the contours from the linked list, into an array for sorting
		//while( (contour_ptr != NULL) ) {
	//		float area = fabs( cv::contourArea(contour_ptr, CV_WHOLE_SEQ) );
	//		if( (area > minArea) && (area < maxArea) ) {
	//            cvSeqBlobs.push_back(contour_ptr);
	//		}
	//		contour_ptr = contour_ptr->h_next;
	//	}
	//

		// sort the pointers based on size
		if( contoursVec.size() > 1 ) {
			sort( contoursVec.begin(), contoursVec.end(), sort_carea_compare );
		}


		// now, we have cvSeqBlobs.size() contours, sorted by size in the array
		// cvSeqBlobs let's get the data out and into our structures that we like
		for( int i = 0; i < MIN(nConsidered, (int)contoursVec.size()); i++ ) {
			blobs.push_back( CvBlob() );
			float area = cv::contourArea( cv::Mat(contoursVec[i]) );
			cv::Rect rect	= cv::boundingRect( cv::Mat(contoursVec[i]) );
			myMoments = cv::moments( cv::Mat(contoursVec[i]) );

			blobs[i].area                     = fabs(area);
			blobs[i].hole                     = area < 0 ? true : false;
			blobs[i].length 			      = cv::arcLength(cv::Mat(contoursVec[i]), true);
			blobs[i].boundingRect = Rectf(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);;
			blobs[i].centroid.x 			  = (myMoments.m10 / myMoments.m00);
			blobs[i].centroid.y 			  = (myMoments.m01 / myMoments.m00);

			// get the points for the blob:

			for( int j=0; j < contoursVec[i].size(); j++ ) {
				blobs[i].pts.push_back( Vec2f(contoursVec[i][j].x, contoursVec[i][j].y) );
			}
			blobs[i].nPts = blobs[i].pts.size();

		}

		nBlobs = blobs.size();

		// Free the storage memory.
		// Warning: do this inside this function otherwise a strange memory leak
		//if( contour_storage != NULL ) { cvReleaseMemStorage(&contour_storage); }
		//if( storage != NULL ) { cvReleaseMemStorage(&storage); }

		return nBlobs;

	}

	//--------------------------------------------------------------------------------
	void CvContourFinder::draw( float x, float y, float w, float h ) {

		float scalex = 0.0f;
		float scaley = 0.0f;
		if( _width != 0 ) { scalex = w/_width; } else { scalex = 1.0f; }
		if( _height != 0 ) { scaley = h/_height; } else { scaley = 1.0f; }

		if(bAnchorIsPct){
			x -= anchor.x * w;
			y -= anchor.y * h;
		}else{
			x -= anchor.x;
			y -= anchor.y;
		}

		// ---------------------------- draw the bounding rectangle
		
		gl::color(Color(.8f, .0f, .7f));

		glPushMatrix();
		glTranslatef( x, y, 0.0 );
		glScalef( scalex, scaley, 0.0 );


		gl::enableWireframe();
		
		for( int i=0; i<(int)blobs.size(); i++ ) {
			gl::drawSolidRect( blobs[i].boundingRect );
		}

		// ---------------------------- draw the blobs

		gl::color(Color(1.0f, .0f, .0f));

		for( int i=0; i<(int)blobs.size(); i++ ) {

			glBegin(GL_TRIANGLE_STRIP);
			for( int j=0; j<blobs[i].nPts; j++ ) {
				gl::vertex( blobs[i].pts[j]);
			}
			glEnd();

		}
		glPopMatrix();
		
		gl::disableWireframe();
	}

	//--------------------------------------------------------------------------------
	void CvContourFinder::setAnchorPercent( float xPct, float yPct ){
		anchor.x = xPct;
		anchor.y = yPct;
		bAnchorIsPct = true;
	}

	//--------------------------------------------------------------------------------
	void CvContourFinder::setAnchorPoint( int x, int y ){
		anchor.x = x;
		anchor.y = y;
		bAnchorIsPct = false;
	}

	//--------------------------------------------------------------------------------
	void CvContourFinder::resetAnchor(){
		anchor.set(0,0);
		bAnchorIsPct = false;
	}

} // namespace cinder

