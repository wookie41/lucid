#version 420 core

layout(location = 0) out vec4 outColor;

in vec3 inNearPoint;
in vec3 inFarPoint;

uniform mat4 uView;
uniform mat4 uProjection;

uniform float uNearPlane;
uniform float uFarPlane;

vec4 Grid(vec3 InFragPos, float InScale)
{
    vec2  Coord      = InFragPos.xz * InScale; // use the scale variable to set the distance between the lines
    vec2  Derivative = fwidth(Coord);
    vec2  Grid       = abs(fract(Coord - 0.5) - 0.5) / Derivative;
    float Line       = min(Grid.x, Grid.y);
    float MinimumZ   = min(Derivative.y, 1);
    float MinimumX   = min(Derivative.x, 1);
    vec4  Color      = vec4(0.2, 0.2, 0.2, 1.0 - min(Line, 1.0));

    // z axis
    if (InFragPos.x > -1 * MinimumX && InFragPos.x < 1 * MinimumX)
        Color.b = 1.0;

    // x axis
    if (InFragPos.z > -1 * MinimumZ && InFragPos.z < 1 * MinimumZ)
        Color.r = 1.0;

    return Color;
}

void main()
{
    float t       = -inNearPoint.y / (inFarPoint.y - inNearPoint.y);
    vec3  FragPos = inNearPoint + t * (inFarPoint - inNearPoint);

    vec4  ClipPos        = uProjection * uView * vec4(FragPos, 1);
    float FragDepth      = ClipPos.z / ClipPos.w;
    float ClipSpaceDepth = FragDepth * 2.0 - 1.0;
    float LinearDepth    = (2.0 * uNearPlane * uFarPlane) / (uFarPlane + uNearPlane - ClipSpaceDepth * (uFarPlane - uNearPlane));
    float Fading         = LinearDepth / uFarPlane;
    
    vec4 GridColor = Grid(FragPos, 1);
    GridColor.a /= Fading;
    
    gl_FragDepth         = FragDepth;
    outColor             =  GridColor * float(t > 0);
}