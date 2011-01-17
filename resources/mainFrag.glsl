#version 110
varying float depth;
varying vec4 vColor;
uniform float lo, hi;

void main()
{
//	if( depth < lo || depth > hi ) discard;

//	gl_FragColor.rgb	= vec3( depth, depth, depth );
//	gl_FragColor.a		= 1.0;

	gl_FragColor = vec4(depth, depth, depth, 1.0);
}





