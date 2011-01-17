#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "Resources.h"
#include "CinderOpenCv.h"
#include "Kinect.h"
#include "CvBlobTracker.h"
#include "CvContourFinder.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class kinectBlobsApp : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void mouseUp( MouseEvent event );
	void update();
	void draw();
	
	Kinect			mKinect;
	gl::Texture		mColorTexture, mDepthTexture;	
	float			mKinectTilt;
	float			mLo, mHi;
	params::InterfaceGl		mParams;
	Surface8u		depthSurf;
	
	int mContMinArea, mContMaxArea, mContMaxBlobs;
	
	CvBlobTracker blobTracker;
	CvContourFinder contourFinder;
};

void kinectBlobsApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 480 );
}

void kinectBlobsApp::setup()
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;

	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected
	
	mParams = params::InterfaceGl( "KinectSkel", Vec2i( 200, 180 ) );
	mParams.addParam( "Kinect Tilt", &mKinectTilt, "min=-31 max=31 keyIncr=T keyDecr=t" );
	mParams.addParam( "lo thresh", &mLo, "min=.0 max=1.0 step=.025 keyIncr=L keyDecr=l" );
	mParams.addParam( "hi thresh", &mHi, "min=.0 max=1.0 step=.025 keyIncr=H keyDecr=h" );
	mParams.addParam( "blob min area", &mContMinArea, "min=0 max=100000 step=10000 keyIncr=N keyDecr=n" );
	mParams.addParam( "blob max area", &mContMaxArea, "min=10000 max=500000 step=10000 keyIncr=M keyDecr=M" );
	mParams.addParam( "num blobs", &mContMaxBlobs, "min=1 max=20 step=1 keyIncr=B keyDecr=b" );
	mContMinArea = 3000;
	mContMaxArea = 300000;
	mContMaxBlobs = 2;
	mLo = 0.6;
	mHi = 1.0;
	mKinectTilt = .0f;
	mDepthTexture	= gl::Texture( 640, 480 );
	mColorTexture	= gl::Texture( 640, 480 );
	depthSurf = Surface8u(640, 480, true);

}

void kinectBlobsApp::mouseUp( MouseEvent event )
{
	writeImage( getHomeDirectory() + "kinect_color.png", mKinect.getColorImage() );
	writeImage( getHomeDirectory() + "kinect_depth.png", mKinect.getDepthImage() );
	
	// set tilt to random angle
	

	mKinect.setLedColor( Kinect::LED_YELLOW );
}

void kinectBlobsApp::update()
{	
	if( mKinect.checkNewDepthFrame() )
	{
		//mDepthTexture = mKinect.getDepthImage();
		depthSurf = Surface8u(mKinect.getDepthImage());
		
		
		
		cv::Mat input( toOcv( depthSurf ) );
		
		cv::Mat thr1, thr2;
	
		cv::threshold( input, thr1, mLo * 255.0f, 255, CV_THRESH_TOZERO );
		cv::threshold( thr1, thr2, mHi * 255.0f, 255, CV_THRESH_TOZERO_INV );
		
		Surface binary(fromOcv(thr2));
		
		
		contourFinder.findContours(binary,
								   mContMinArea,
								   mContMaxArea,
								   mContMaxBlobs,
								   false,
								   true);
		
		blobTracker.trackBlobs(contourFinder.blobs);
		
		Surface8u outSurf(fromOcv(thr2));

		mDepthTexture = gl::Texture( outSurf );
		mColorTexture = gl::Texture( outSurf );
	}

	
	if( mKinectTilt != mKinect.getTilt() )
		mKinect.setTilt( mKinectTilt );
	

}

void kinectBlobsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	
	gl::translate(Vec2f(1280,0));
	gl::scale(Vec3f(-1.0f, 1.0f, 1.0f));
	
	if( mDepthTexture )
		gl::draw( mDepthTexture );
	
//	if( mColorTexture )
//		gl::draw( mColorTexture, Vec2i( 640, 0 ) );

	//blobTracker.draw(640,0);
	
	gl::translate(Vec2f(640, 0));
	
	vector<CvTrackedBlob>::iterator blob_it;
	Color colors[] = {
		Color(1.0f, .0f, .0f),
		Color(.0f, 1.0f, .0f),
		Color(.0f, .0f, 1.0f)
	};
	
	int c = 0;
	for(blob_it = blobTracker.blobs.begin(); blob_it < blobTracker.blobs.end(); blob_it++, c++)
	{
		CvTrackedBlob &blob = *blob_it;
		gl::color(colors[c%3]);
		gl::drawSolidCircle(blob.centroid, 30.0f, 64);
	}
	
	params::InterfaceGl::draw();
}


CINDER_APP_BASIC( kinectBlobsApp, RendererGl )
