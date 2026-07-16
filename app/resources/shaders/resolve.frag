#version 330 core
in vec2  vUV;
out vec4 FragColor;

uniform sampler2D uPhysicalTex;
uniform sampler1D uColorMap;
uniform float     uRangeMin;
uniform float     uRangeMax;
uniform bool      uUseLogScale;
uniform float     uGamma;
uniform float     uOpacity;
uniform float     uScaleFactor;

void main() {
    vec4  data  = texture(uPhysicalTex, vUV);
    float value = data.r;
    float Wsum  = data.g;

    if (Wsum < 1e-12)
        discard;

    float phisValue     = value * uScaleFactor;
    float safePhisValue = max(1e-12, phisValue); // Защита от log(0)
    float INV_LOG10     = 1.0 / log(10.0);

    float correctValue = uUseLogScale ? log(safePhisValue) * INV_LOG10 : phisValue;

    float physMinBoundary = uRangeMin * uScaleFactor;
    float physMaxBoundary = uRangeMax * uScaleFactor;

    float phisRangeMin =
        uUseLogScale ? log(max(1e-12, physMinBoundary)) * INV_LOG10 : physMinBoundary;
    float phisRangeMax =
        uUseLogScale ? log(max(1e-12, physMaxBoundary)) * INV_LOG10 : physMaxBoundary;


    float t =
        clamp((correctValue - phisRangeMin) / max(1e-6, phisRangeMax - phisRangeMin), 0.0, 1.0);
    t = pow(t, 1.0 / max(0.01, uGamma));

    FragColor = vec4(texture(uColorMap, t).rgb, uOpacity);
}