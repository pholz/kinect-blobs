/*
 *  CvBlob.h
 *  kinectBlobs
 *
 *  Created by Peter Holzkorn on 17/01/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "cinder/gl/gl.h"
#include <vector>

using namespace std;

namespace cinder {

	class CvBlob {
		
	public:
		
		float               area;
		float               length;
		Rectf				boundingRect;
		Vec2f				centroid;
		bool                hole;
		
		vector<Vec2f>		pts;    // the contour of the blob
		int                 nPts;   // number of pts;
		
		//----------------------------------------
		CvBlob() {
			area 		= 0.0f;
			length 		= 0.0f;
			hole 		= false;
			nPts        = 0;
		}
		
		//----------------------------------------
		void draw(float x = 0, float y = 0){
			
			gl::enableWireframe();
			gl::color(Color(.0f, 1.0f, 1.0f));
						
			glBegin(GL_TRIANGLE_STRIP);
			
			for (int i = 0; i < nPts; i++){
				gl::vertex(Vec2f(x + pts[i].x, y + pts[i].y));
			}
			
			glEnd();

			//gl::drawofRect(x + boundingRect.x, y + boundingRect.y, boundingRect.width, boundingRect.height);
			
			gl::disableWireframe();
		}
	};

}