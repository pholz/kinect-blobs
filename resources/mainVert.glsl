uniform sampler2D depthTex;
//uniform sampler2D colorTex;
uniform float lo, hi;
varying vec4 vVertex;
varying vec4 vColor;
varying float depth;

void main()
{


	gl_Position			= gl_ModelViewProjectionMatrix * gl_Vertex;
}
