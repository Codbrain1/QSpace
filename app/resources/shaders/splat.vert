#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aScalar;

uniform mat4  uMVP;
uniform float uPointScale;
uniform float uWorldRadius;

out float vScalar;

void main() {
    gl_Position  = uMVP * vec4(aPos, 1.0);
    gl_PointSize = 4.0 * uPointScale * uWorldRadius;
    vScalar      = aScalar;
}