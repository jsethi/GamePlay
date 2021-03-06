#ifdef OPENGL_ES
precision highp float;
#endif

// Uniforms
uniform vec3 u_lightColor;                      // Light color
uniform vec3 u_ambientColor;                    // Ambient color
uniform vec4 u_diffuseColor;                    // Diffuse color
#if defined(GLOBAL_ALPHA)
uniform float u_globalAlpha;                    // Global alpha value
#endif

// Inputs
varying vec3 v_normalVector;                    // NormalVector in view space.

// Global variables
vec4 _baseColor;                                // Base color
vec3 _ambientColor;                             // Ambient Color
vec3 _diffuseColor;                             // Diffuse Color

void lighting(vec3 normalVector, vec3 lightDirection, float attenuation)
{
    // Ambient
    _ambientColor = _baseColor.rgb * u_ambientColor;

    // Diffuse
    float ddot = dot(normalVector, lightDirection);
    float diffuseIntensity = attenuation * ddot;
    diffuseIntensity = max(0.0, diffuseIntensity);
    _diffuseColor = u_lightColor * _baseColor.rgb * diffuseIntensity;
}

#if defined(POINT_LIGHT)

varying vec4 v_vertexToPointLightDirection;     // Light direction w.r.t current vertex.

void applyLight()
{
    // Normalize the vectors.
    vec3 normalVector = normalize(v_normalVector);
    
    vec3 vertexToPointLightDirection = normalize(v_vertexToPointLightDirection.xyz);
    
    // Fetch point light attenuation.
    float pointLightAttenuation = v_vertexToPointLightDirection.w;
    lighting(normalVector, vertexToPointLightDirection, pointLightAttenuation);
}

#elif defined(SPOT_LIGHT)

uniform vec3 u_spotLightDirection;              // Direction of the spot light.
uniform float u_spotLightInnerAngleCos;         // The bright spot [0.0 - 1.0]
uniform float u_spotLightOuterAngleCos;         // The soft outer part [0.0 - 1.0]
varying vec3 v_vertexToSpotLightDirection;      // Light direction w.r.t current vertex.
varying float v_spotLightAttenuation;           // Attenuation of spot light.

float lerpstep( float lower, float upper, float s)
{
    return clamp( ( s - lower ) / ( upper - lower ), 0.0, 1.0 );
}

void applyLight()
{
    // Normalize the vectors.
    vec3 normalVector = normalize(v_normalVector);
    vec3 spotLightDirection = u_spotLightDirection; 
    vec3 vertexToSpotLightDirection = normalize(v_vertexToSpotLightDirection);

    // "-lightDirection" is used because light direction points in opposite direction to
    // to spot direction.
    // Calculate spot light effect.
    float spotCurrentAngleCos = max(0.0, dot(spotLightDirection, -vertexToSpotLightDirection));

    // Intensity of spot depends on the spot light attenuation and the 
    // part of the cone vertexToSpotLightDirection points to (inner or outer).
    float spotLightAttenuation = clamp(v_spotLightAttenuation, 0.0, 1.0);
    spotLightAttenuation *= lerpstep(u_spotLightOuterAngleCos, u_spotLightInnerAngleCos, spotCurrentAngleCos);

    lighting(normalVector, vertexToSpotLightDirection, spotLightAttenuation);
}

#else

uniform vec3 u_lightDirection;       	        // Light direction

void applyLight()
{
    // Normalize the vectors.
    vec3 normalVector = normalize(v_normalVector);
    vec3 lightDirection = normalize(u_lightDirection);

    lighting(normalVector, -lightDirection, 1.0);
}
#endif

void main()
{
    // Fetch diffuse color from texture.
    _baseColor = u_diffuseColor;

    // Apply light
    applyLight();

    // Light the pixel
    gl_FragColor.a = _baseColor.a;
    gl_FragColor.rgb = _ambientColor + _diffuseColor;

#if defined(GLOBAL_ALPHA)
    gl_FragColor.a *= u_globalAlpha;
#endif
}