#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 FragPos;
layout(location = 1) smooth in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in float Light;

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 normal) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    // combine results
    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse  * diff;
    return (ambient + diffuse);
}

void main() {
    if (Light == 0. || Normal == vec3(0.0, 0.0, 0.0)) {
        outColor = Color;
    } else {
        DirLight dirLight;
        dirLight.direction = vec3(1., 1., 1.);
        dirLight.ambient = vec3(0.3, 0.3, 0.3);
        dirLight.diffuse = vec3(0.5, 0.5, 0.5);
        vec3 lights = CalcDirLight(dirLight, normalize(Normal));
        outColor = vec4(lights, 1.0) * Color;
    }
}
