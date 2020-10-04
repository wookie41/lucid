#version 330 core

in vec3 Normal;
in vec3 WorldPos;

out vec4 FragColor;

uniform float uAmbientStrength;

// Shader-wide properties

uniform vec3 uLightPos;
uniform vec3 uViewPos;

// Material properties

uniform vec3 uDiffuseColor;
uniform vec3 uSpecularColor;
uniform float uShininess;

void main()
{
    // normalize the interpolated normal
    vec3 normal = normalize(Normal);

    vec3 toLight = normalize(uLightPos - WorldPos);
    vec3 toView = normalize(uViewPos - WorldPos);

    vec3 ambient = uDiffuseColor * uAmbientStrength;

    float diffuseStrength = max(dot(toLight, Normal), 0.0);
    vec3 diffuse = uDiffuseColor * diffuseStrength;

    vec3 reflectedToLight = reflect(-toLight, normal);
    float specularStrength = pow(max(dot(toView, reflectedToLight), 0.0), uShininess);
    vec3 specular = specularStrength * uSpecularColor * uDiffuseColor; 

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
