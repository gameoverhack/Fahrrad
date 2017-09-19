/*precision mediump float;
#define IN varying
#define OUT
#define TEXTURE texture2D
#define FRAG_COLOR gl_FragColor
#define TARGET_OPENGLES
#define SAMPLER sampler2D

uniform float brightness;
uniform float contrast;
uniform float saturation;
uniform SAMPLER Ytex;
uniform SAMPLER Utex;
uniform SAMPLER Vtex;
uniform vec2 tex_scaleY;
uniform vec2 tex_scaleU;
uniform vec2 tex_scaleV;
uniform vec4 globalColor;
IN vec4 colorVarying;
IN vec2 texCoordVarying;
const vec3 offset = vec3(-0.0625, -0.5, -0.5);
const vec3 rcoeff = vec3(1.164, 0.000, 1.596);
const vec3 gcoeff = vec3(1.164,-0.391,-0.813);
const vec3 bcoeff = vec3(1.164, 2.018, 0.000);
*/

#version 150

uniform sampler2DRect tex;

uniform float brightness;
uniform float contrast;
uniform float saturation;

uniform vec4 globalColor;

in vec2 vTexCoord;

out vec4 vFragColor;

mat4 brightnessMatrix(float brightness)
{
    return mat4(1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                brightness, brightness, brightness, 1);
}

mat4 contrastMatrix(float contrast)
{
    float t =  (1.0 - contrast)/2.0;
    
    return mat4(contrast, 0, 0, 0,
                0, contrast, 0, 0,
                0, 0, contrast, 0,
                t, t, t, 1);
}

mat4 saturationMatrix(float saturation)
{
    vec3 luminance = vec3(0.3086, 0.6094, 0.082);
    
    float oneMinusSat = 1.0 - saturation;
    
    vec3 red = vec3(luminance.x*oneMinusSat);
    red += vec3(saturation, 0, 0);
    
    vec3 green = vec3(luminance.y*oneMinusSat);
    green += vec3(0, saturation, 0);
    
    vec3 blue = vec3(luminance.z*oneMinusSat);
    blue += vec3(0, 0, saturation);
    
    return mat4(red, 0,
                green, 0,
                blue, 0,
                0, 0, 0, 1);
}

/*
void main(){
    vec3 yuv;
    yuv.x=TEXTURE(Ytex,texCoordVarying * tex_scaleY).r;
    yuv.y=TEXTURE(Utex,texCoordVarying * tex_scaleU).r;
    yuv.z=TEXTURE(Vtex,texCoordVarying * tex_scaleV).r;
    yuv += offset;
    
    vec4 col;
    
    col.r = dot(yuv, rcoeff);
    col.g = dot(yuv, gcoeff);
    col.b = dot(yuv, bcoeff);
    col.a = 1.0;
    
    //FRAG_COLOR = brightnessMatrix(brightness) * contrastMatrix(contrast) * saturationMatrix(saturation) * col * globalColor;
    FRAG_COLOR = col * globalColor;
}
*/

void main(){

    vec4 col = texture(tex, vTexCoord);
    
    vFragColor = brightnessMatrix(brightness) * contrastMatrix(contrast) * saturationMatrix(saturation) * col * globalColor;
    
}