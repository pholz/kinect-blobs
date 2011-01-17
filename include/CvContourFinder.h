/*
* ofxCvContourFinder.h
*
* Finds white blobs in binary images and identifies
* centroid, bounding box, area, length and polygonal contour
* The result is placed in a vector of ofxCvBlob objects.
*
*/


#include "CvConstants_Track.h"
#include "CvBlob.h"
#include <algorithm>

namespace cinder {
	

	class CvContourFinder {

	  public:
	  
		vector<CvBlob>  blobs;
		int                nBlobs;    // DEPRECATED: use blobs.size() instead
		  

		CvContourFinder();
		virtual  ~CvContourFinder();
		
		virtual float getWidth() { return _width; };    //set after first findContours call
		virtual float getHeight() { return _height; };  //set after first findContours call
		
		virtual int  findContours( Surface& input,
								   int minArea, int maxArea,
								   int nConsidered, bool bFindHoles,
								   bool bUseApproximation = true);
								   // approximation = don't do points for all points 
								   // of the contour, if the contour runs
								   // along a straight line, for example...

		virtual void  draw() { draw(0,0, _width, _height); };
		virtual void  draw( float x, float y ) { draw(x,y, _width, _height); };
		virtual void  draw( float x, float y, float w, float h );
		virtual void setAnchorPercent(float xPct, float yPct);
		virtual void setAnchorPoint(int x, int y);
		virtual void resetAnchor();      
		//virtual ofxCvBlob  getBlob(int num);



	  protected:

		int  _width;
		int  _height;
		Surface					inputCopy;
		
		std::vector< std::vector<cv::Point> > contoursVec;
		
	//	CvMemStorage*           contour_storage;
	//	CvMemStorage*           storage;
		cv::Moments              myMoments;
	//	vector<CvSeq*>          cvSeqBlobs;  //these will become blobs
		
		Vec2f  anchor;
		bool  bAnchorIsPct;      

		virtual void reset();

	};



}
