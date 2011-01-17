/*
 *  CvConstants_Track.h
 *  kinectBlobs
 *
 *  Created by Peter Holzkorn on 17/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
#include <iostream>
#include "CvBlob.h"
#include "CvTrackedBlob.h"


#define  MAX_NUM_CONTOURS_TO_FIND   128  // alther this if you think you will
// be looking for more....
#define  MAX_CONTOUR_LENGTH        1024  // alther this if you think your
// contours will be longer than this

namespace cinder {

	class CvBlobListener {
	public:
		
		virtual void blobOn( int x, int y, int id, int order ) = 0;
		virtual void blobMoved( int x, int y, int id, int order ) = 0;
		virtual void blobOff( int x, int y, int id, int order ) = 0;
		
	};

}