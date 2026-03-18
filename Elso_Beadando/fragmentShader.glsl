#version 330

uniform vec2 center;
uniform float radius;

uniform float offsetX;
uniform float offsetY;
uniform vec2 windowSize;

uniform float lineY;

out vec4 outColor;

void main()
{
    vec3 red = vec3(1.0, 0.0, 0.0);
    vec3 green = vec3(0.0, 1.0, 0.0);
    vec2 px = gl_FragCoord.xy;

    vec2 movedCenter = center + vec2(offsetX, offsetY);
    float dist = distance(px, movedCenter);

    float thirdLength = windowSize.x / 6.0;
    float lineThickness = 1.5;

    bool circleCrossesLine = abs(movedCenter.y - (center.y + lineY)) <= radius;
    if (!circleCrossesLine) {
        vec3 tmp = red;
        red = green;
        green = tmp;
    }

    if(dist <= radius) {
        float t = dist / radius;
        vec3 color = mix(red, green, t);

        outColor = vec4(color, 1.0);
    }
    else if(abs(px.y - (center.y + lineY)) <= lineThickness && abs(px.x - center.x) <= thirdLength) {
        outColor = vec4(0.0,0.0,1.0,1.0);
    }
    else {
        outColor = vec4(1.0, 1.0, 0.0, 1.0);
    }
}