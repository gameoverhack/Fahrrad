/*attribute vec4 position;
attribute vec4 color;
attribute vec4 normal;
attribute vec2 texcoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

varying vec4 colorVarying;
varying vec2 texCoordVarying;


void main()
{
	vec4 pos = projectionMatrix * modelViewMatrix * position;
	gl_Position = pos;

	colorVarying = color;
	texCoordVarying = texcoord;
}


void main(void) {
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
*/

precision mediump float;
#define IN attribute
#define OUT varying
#define TEXTURE texture2D
#define TARGET_OPENGLES
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

IN vec4 position;
IN vec2 texcoord;
IN vec4 color;
IN vec3 normal;
OUT vec4 colorVarying;
OUT vec2 texCoordVarying;
OUT vec4 normalVarying;
void main() {
    colorVarying = color;
    texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
    gl_Position = modelViewProjectionMatrix * position;
}
