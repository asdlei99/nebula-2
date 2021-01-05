//------------------------------------------------------------------------------
//  brightpass.fx
//  (C) 2012 Gustav Sterbrant
//------------------------------------------------------------------------------

#include "lib/std.fxh"
#include "lib/util.fxh"
#include "lib/shared.fxh"
#include "lib/techniques.fxh"


/// Declaring used textures
texture2D ColorSource;
texture2D LuminanceTexture;

sampler_state BrightPassSampler
{
    //Samplers = { ColorSource, LuminanceTexture };
    Filter = Point;
};

render_state BrightPassState
{
    CullMode = Back;
    DepthEnabled = false;
    DepthWrite = false;
};


//------------------------------------------------------------------------------
/**
*/
shader
void
vsMain(
    [slot=0] in vec3 position,
    [slot=2] in vec2 uv,
    out vec2 UV) 
{
    gl_Position = vec4(position, 1);
    UV = uv;
}

//------------------------------------------------------------------------------
/**
*/
shader
void
psMain(in vec2 uv,
    [color0] out vec4 Color) 
{   
    vec4 sampleColor = textureLod(sampler2D(ColorSource, BrightPassSampler), uv, 0);
    
    // Get the calculated average luminance 
    //float lumavg = texelFetch(sampler2D(LuminanceTexture, BrightPassSampler), ivec2(0, 0), 0).r;
    //vec3 brightColor = ToneMap(sampleColor, vec4(lumavg), 1).rgb;

    float pixelLuminance = dot(sampleColor, vec4(0.2126f, 0.7152f, 0.0722f, 0));
    vec3 brightColor = mix(vec3(0.0f), sampleColor.rgb, clamp(pixelLuminance - HDRBrightPassThreshold, 0.0f, 1.0f));
    Color = HDRBloomColor * vec4(brightColor, sampleColor.a);
}

//------------------------------------------------------------------------------
/**
*/
PostEffect(vsMain(), psMain(), BrightPassState);
