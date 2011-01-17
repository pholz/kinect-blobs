/*
 *  CvBlobTracker.h
 *  kinectBlobs
 *
 *  Created by Peter Holzkorn on 17/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <map>
#include <vector>
//#include "ofMain.h"
//#include "ofAddons.h"

#include "CvTrackedBlob.h"
#include "CvConstants_Track.h"

using namespace std;

namespace cinder {
	
	class CvBlobTracker {
		
		
	public:
		
		vector<CvTrackedBlob>  blobs;
		
		
		CvBlobTracker();
		void setListener( CvBlobListener * _listener );
		void trackBlobs( const vector<CvBlob>& blobs );
		int findOrder( int id );  // order by which the present
		// blobs came into existence
		CvTrackedBlob& getById( int id );  // returns a reference to the
		// corresponding blob in blobs vector
		void draw( float x, float y );  // draws all blobs
		
		
		
	protected:
		
		int currentID;
		int extraIDs;
		int numcheck;
		
		CvBlobListener* listener;
		
		int reject_distance_threshold;
		int ghost_frames;
		float minimumDisplacementThreshold;
		
		vector<vector<int> > matrix;
		vector<int> ids;
		vector<vector<CvTrackedBlob> > history;
		
		
		void doBlobOn( const CvTrackedBlob& b );
		void doBlobMoved( const CvTrackedBlob& b );
		void doBlobOff( const CvTrackedBlob& b );
		
		inline void permute( int k );
		inline bool checkValid( int start );
		inline bool checkValidNew( int start );
	};

}