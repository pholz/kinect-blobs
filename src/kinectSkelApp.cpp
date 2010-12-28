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

using namespace ci;
using namespace ci::app;
using namespace std;

static float L0[]={
	-1,-1,-1,-1,-1,
	0, 0, 0, 0, 0,
	2, 2, 2, 2, 2,
	0, 0, 0, 0, 0,
	-1,-1,-1,-1,-1
};
static float L45[]={
	0,-1,-1, 0, 2,
	-1,-1, 0, 2, 0,
	-1, 0, 2, 0,-1,
	0, 2, 0,-1,-1,
	2, 0,-1,-1, 0
};
static float L90[]={
	-1, 0, 2, 0,-1,
	-1, 0, 2, 0,-1,
	-1, 0, 2, 0,-1,
	-1, 0, 2, 0,-1,
	-1, 0, 2, 0,-1
};
static float L135[]={
	2, 0,-1,-1, 0,
	0, 2, 0,-1,-1,
	-1, 0, 2, 0,-1,
	-1,-1, 0, 2, 0,
	0,-1,-1, 0, 2
};

float getMax(float v1, float v2, float v3, float v4){
	float max1 = v1 > v2 ? v1 : v2;
	float max2 = v3 > v4 ? v3 : v4;
	return max1 > max2 ? max1 : max2;
}

class kinectSkelApp : public AppBasic {
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
	int				mSkelThresh;
	int			mDilateSize, mDilateIter;
	params::InterfaceGl		mParams;
	gl::Fbo			mFboThresh;
	gl::GlslProg	mShader;
	Surface8u		depthSurf;
	cv::Mat kern00, kern45, kern90, kern135;
};

void kinectSkelApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 480 );
}

void kinectSkelApp::setup()
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;

	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected
	
	mParams = params::InterfaceGl( "KinectSkel", Vec2i( 200, 180 ) );
	mParams.addParam( "Kinect Tilt", &mKinectTilt, "min=-31 max=31 keyIncr=T keyDecr=t" );
	mParams.addParam( "lo thresh", &mLo, "min=.0 max=1.0 step=.025 keyIncr=L keyDecr=l" );
	mParams.addParam( "hi thresh", &mHi, "min=.0 max=1.0 step=.025 keyIncr=H keyDecr=h" );
	mParams.addParam( "dilate size", &mDilateSize, "min=3 max=20 step=1 keyIncr=D keyDecr=d" );
	mParams.addParam( "dilate iter", &mDilateIter, "min=1 max=10 step=1 keyIncr=F keyDecr=f" );
	mParams.addParam( "skel thresh", &mSkelThresh, "min=0 max=255 step=1 keyIncr=S keyDecr=s" );
	mLo = 0.5;
	mHi = 1.0;
	mDilateSize = 8;
	mDilateIter = 1;
	mKinectTilt = .0f;
	mSkelThresh = 0;
	mFboThresh = gl::Fbo(640, 480);
	mShader	= gl::GlslProg( loadResource( RES_VERT_ID ), loadResource( RES_FRAG_ID ) );
	mDepthTexture	= gl::Texture( 640, 480 );
	mColorTexture	= gl::Texture( 640, 480 );
	depthSurf = Surface8u(640, 480, true);
	
	kern00 = cv::Mat(5, 5, CV_32FC1, L0);
	kern45 = cv::Mat(5, 5, CV_32FC1, L45);
	kern90 = cv::Mat(5, 5, CV_32FC1, L90);
	kern135 = cv::Mat(5, 5, CV_32FC1, L135);
}

void kinectSkelApp::mouseUp( MouseEvent event )
{
	writeImage( getHomeDirectory() + "kinect_color.png", mKinect.getColorImage() );
	writeImage( getHomeDirectory() + "kinect_depth.png", mKinect.getDepthImage() );
	
	// set tilt to random angle
	

	mKinect.setLedColor( Kinect::LED_YELLOW );
}

void kinectSkelApp::update()
{	
	if( mKinect.checkNewDepthFrame() )
	{
		//mDepthTexture = mKinect.getDepthImage();
		depthSurf = Surface8u(mKinect.getDepthImage());
		
		cv::Mat input( toOcv( depthSurf ) );
		cv::Mat thr1, thr2, dilated, thr3(640, 480, CV_8UC1), dist(640, 480, CV_32FC1), distC, distLapl(640, 480, CV_8UC1),
		distLapl00(640, 480, CV_8UC1), distLapl45(640, 480, CV_8UC1), distLapl90(640, 480, CV_8UC1), distLapl135(640, 480, CV_8UC1), distLaplC, distThresh, distThreshC, distBlur, distFinal, distFinalC;
		
		//cv::medianBlur( input, output, 100 );
		//	cv::Sobel( input, output, CV_8U, 0, 1 ); 
		cv::threshold( input, thr1, mLo * 255.0f, 255, CV_THRESH_TOZERO );
		cv::threshold( thr1, thr2, mHi * 255.0f, 255, CV_THRESH_TOZERO_INV );
		//cv::dilate( thr2, dilated, cv::Mat(mDilateSize, mDilateSize, CV_8UC1), cv::Point(-1,-1), mDilateIter );
		cv::morphologyEx(thr2, dilated, cv::MORPH_CLOSE, cv::Mat(mDilateSize, mDilateSize, CV_8UC1), cv::Point(-1,-1), mDilateIter);
		cv::threshold( dilated, thr3, mLo * 255.0f, 255, CV_THRESH_BINARY );
		cv::Mat thr3bin( toOcv( Channel8u( fromOcv( thr3 ) ) ) );
		cv::distanceTransform(thr3bin, dist, CV_DIST_L2, CV_DIST_MASK_5);
		//cv::cvtColor(dist, distC, CV_GRAY2RGB);
		dist.convertTo(distC, CV_8UC1, 1.0f, .0f);
		//cv::Laplacian(distC, distLapl, 8, 3);
	//	cv::Sobel(distC, distLapl, 8, 1, 1);
		
		cv::filter2D(distC, distLapl00, 8, kern00, cv::Point(-1, -1), 0);
		cv::filter2D(distC, distLapl45, 8, kern45, cv::Point(-1, -1), 0);
		cv::filter2D(distC, distLapl90, 8, kern90, cv::Point(-1, -1), 0);
		cv::filter2D(distC, distLapl135, 8, kern135, cv::Point(-1, -1), 0);
		
		distLapl = distLapl.t();
		
		int index;
		for(int x = 0; x < 640; x++)
		{
			for(int y = 0; y < 480; y++)
			{
				index = y * 640 + x;
				float smax = getMax(distLapl00.data[index], distLapl45.data[index], distLapl90.data[index], distLapl135.data[index]);
				distLapl.data[index] = smax > 0 ? smax : 0;
			}
		}
		
		//distLapl = distLapl.t();
		
		distLapl.convertTo(distLaplC, CV_8UC1, 1.0f, .0f);
		cv::threshold(distLaplC, distThresh, mSkelThresh, 255, CV_THRESH_BINARY);
	//	cv::adaptiveThreshold(distLaplC, distThresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 7, 0);
		
		distThresh.convertTo(distThreshC, CV_8UC1, 1.0f, .0f);
		
		cv::GaussianBlur(distThreshC, distBlur, cv::Size(5, 5), 3, 0);
		cv::adaptiveThreshold(distBlur, distFinal, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 7, 0);
		//cv::threshold(distBlur, distFinal, mSkelThresh, 255, CV_THRESH_TOZERO);
		//cv::medianBlur( distThreshC, distFinal, 3 );
		distFinal.convertTo(distFinalC, CV_8UC1, 1.0f, .0f);
		Surface8u surfDist(fromOcv(distThreshC));
		Surface8u surfDistRaw(fromOcv(distC));

		mDepthTexture = gl::Texture( surfDist );
		mColorTexture = gl::Texture( surfDistRaw );
	}
		
	

	
//	if( mKinect.checkNewColorFrame() )
//		mColorTexture = mKinect.getColorImage();
	
	if( mKinectTilt != mKinect.getTilt() )
		mKinect.setTilt( mKinectTilt );
	
//	console() << "Accel: " << kinect.getAccel() << std::endl;
}

void kinectSkelApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	
	//mFboThresh.bindFramebuffer();
	
//	mDepthTexture.bind( 0 );
//	mColorTexture.bind( 1 );
//	mShader.bind();
//	mShader.uniform( "depthTex", 0 );
//	mShader.uniform( "colorTex", 1 );
//	mShader.uniform( "lo", mLo);
//	mShader.uniform( "hi", mHi);
	
	if( mDepthTexture )
		gl::draw( mDepthTexture );

	//glBegin(GL_TRIANGLE_STRIP);
//	
//	glTexCoord2f(.0f, .0f);
//	glVertex2f(0, 0);
//	
//
//	glTexCoord2f(1.0f, .0f);
//		glVertex2f(640, 0);
//	
//	
//	glTexCoord2f(.0f, 1.0f);
//	glVertex2f(0, 480);
//
//	glTexCoord2f(1.0f, 1.0f);
//		glVertex2f(640, 480);
//	
//	glEnd();
//	
//	mShader.unbind();
	
	//mFboThresh.unbindFramebuffer();
	
	if( mColorTexture )
		gl::draw( mColorTexture, Vec2i( 640, 0 ) );
	
	params::InterfaceGl::draw();
}


CINDER_APP_BASIC( kinectSkelApp, RendererGl )
