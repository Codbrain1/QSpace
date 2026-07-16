#version 330 core
in float vScalar;
out vec4 FragColor;

uniform float uWorldRadius;

// сглаживающее ядро учитывает радиус 2h
float cubicSpline(float r, float h) {
    float ss = r / h;
    if (ss >= 2.0)
        return 0.0; // Затухает при 2h!

    float factor = 10.0 / (7.0 * 3.14159265 * h * h);
    float W;

    if (ss < 1.0) {
        W = 1.0 + (0.75 * ss - 1.5) * ss * ss;
    } else {
        float t = 2.0 - ss;
        W       = 0.25 * t * t * t;
    }

    return factor * W;
}

void main() {
    vec2  coord = gl_PointCoord * 2.0 - 1.0;
    float q2    = dot(coord, coord);
    if (q2 > 1.0)
        discard;
    float q = sqrt(q2);
    // размазываем скалярную величину в на сглаживающий радиус
    float normalization = cubicSpline(2.0 * q * uWorldRadius, uWorldRadius);

    FragColor = vec4(vScalar * normalization, normalization, 1.0, 0.0);
}