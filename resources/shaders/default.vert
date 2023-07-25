#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out float outLight;

layout( push_constant ) uniform constants {
    mat4 transformMatrix;
    vec4 color;
    float lightning;
} PushConstants;

void main() {
    gl_Position = PushConstants.transformMatrix*vec4(inPosition, 1.0);
    outPos = inPosition;
    outNormal = inNormal;
    outColor = PushConstants.color;
    outLight = PushConstants.lightning;
}
