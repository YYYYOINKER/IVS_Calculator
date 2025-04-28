#version 330 core
out vec4 FragColor;

uniform float uProgress; // 0.0 to 1.0

void main() {
    if (gl_FragCoord.x / 800.0 > uProgress) {
        discard; // only render up to progress bar width
    }
    FragColor = vec4(0.2, 0.8, 0.3, 1.0); // green
}

