/*
 *  CvTrackedBlob.h
 *  kinectBlobs
 *
 *  Created by Peter Holzkorn on 17/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "CvBlob.h"
#include "CinderOpenCv.h"
#include <vector>

using namespace std;

namespace cinder {
	
	class CvTrackedBlob : public CvBlob {
	public:
		
		int id;
		
		Vec2f deltaLoc;
		Vec2f deltaLocTotal;
		Vec2f predictedPos;
		float deltaArea;
		
		
		// Used only by BlobTracker
		//
		bool markedForDeletion;
		int framesLeft;
		vector<float> error;
		vector<int> closest;  // ids of the closest points, sorted
		
		
		
		CvTrackedBlob() {
			id = -1;
			area = 0.0f;
			length = 0.0f;
			deltaArea = 0.0f;
			markedForDeletion = false;
			framesLeft = 0;
		}
		
		CvTrackedBlob( const CvBlob& b ) {
			area = b.area;
			length = b.length;
			boundingRect = b.boundingRect;
			centroid = b.centroid;
			hole = b.hole;
			pts = b.pts;
			
			id = -1;
			deltaArea = 0.0f;
			markedForDeletion = false;
			framesLeft = 0;
		}
		
		
		
		
		int getLowestError() {
			int best=-1;
			float best_v=99999.0f;
			for( unsigned int i=0; i<error.size(); i++ ) {
				if( error[i] < best_v ) {
					best = i;
					best_v = error[i];
				}
			}
			return best;
		}
		
		
	};

}