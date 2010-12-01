
vec2 textureDriftUV(vec2 texCoordsIn, float t, float speedU, float speedV) {
    float x = texCoordsIn.x + t * speedU;
    float y = texCoordsIn.y - t * speedV;
    return vec2(x, y);
}

vec2 textureDriftU(vec2 texCoordsIn, float t, float speed) {
    vec2 result;
    result.x = texCoordsIn.x + speed * t;
    result.y = texCoordsIn.y;
    return result;
}

vec2 textureDriftV(vec2 texCoordsIn, float t, float speed) {
    vec2 result;
    result.x = texCoordsIn.x;
    result.y = texCoordsIn.y - speed * t;
    return result;
}

vec2 textureSwirl(vec2 texCoordsIn, float t, float speed) {
    float theta = t * speed;

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    mat2 rotMatrix = mat2(
        cosTheta, sinTheta,
        -sinTheta, cosTheta
    );

    vec2 result = rotMatrix * vec2(texCoordsIn.x - 0.5, texCoordsIn.y - 0.5);

    result.x += 0.5;
    result.y += 0.5;

    return result;
}

vec2 textureWaveyUV(vec2 texCoordsIn, float t, float speedU, float speedV) {
    vec2 result;
    result.x = fract(texCoordsIn.x - sin(t * speedU));
    result.y = fract(texCoordsIn.y - sin(t * speedV));
    return result;
}

vec2 textureWaveyU(vec2 texCoordsIn, float t, float speed) {
    vec2 result;
    result.x = fract(texCoordsIn.x - sin(t * speed));
    result.y = texCoordsIn.y;
    return result;
}

vec2 textureWaveyV(vec2 texCoordsIn, float t, float speed) {
    vec2 result;
    result.x = texCoordsIn.x;
    result.y = fract(texCoordsIn.y - sin(t * speed));
    return result;
}
