#version 330 core

struct AABB {
    vec3 min;
    vec3 max;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

out vec4 color;

uniform vec3 backgroundColor;
uniform vec3 octreeColor;
uniform vec3 lightColor;

uniform mat4 cameraMat;
uniform vec3 cameraPos;
uniform vec3 lightPos;

uniform vec3 w0;
uniform vec3 h0;

uniform vec3 stepW;
uniform vec3 stepH;

uniform vec3 aabbMin;
uniform vec3 aabbMax;

//uniform AABB aabb;

Ray constructRay() {
    Ray ray;
    ray.origin = cameraPos;
    ray.direction = normalize(w0 + stepW * gl_FragCoord.x + h0 + stepH * gl_FragCoord.y);
    return ray;
}

bool rayAABBIntersect(in Ray ray, in AABB aabb) {
    float loX = (aabb.min.x - ray.origin.x) / ray.direction.x;
    float hiX = (aabb.max.x - ray.origin.x) / ray.direction.x;

    float tmin = min(loX, hiX);
    float tmax = max(loX, hiX);

    float loY = (aabb.min.y - ray.origin.y) / ray.direction.y;
    float hiY = (aabb.max.y - ray.origin.y) / ray.direction.y;

    tmin = max(tmin, min(loY, hiY));
    tmax = min(tmax, max(loY, hiY));

    float loZ = (aabb.min.z - ray.origin.z) / ray.direction.z;
    float hiZ = (aabb.max.z - ray.origin.z) / ray.direction.z;

    tmin = max(tmin, min(loZ, hiZ));
    tmax = min(tmax, max(loZ, hiZ));

    return (tmin <= tmax) && (tmax > 0.0f);
}

vec4 castRay(in Ray ray) {
    AABB aabb;
    aabb.min = aabbMin;
    aabb.max = aabbMax;
    if (rayAABBIntersect(ray, aabb)) {
        return vec4(octreeColor, 1.0);
    } else {
        return vec4(backgroundColor, 1.0);
    }
}

void main() {
    Ray ray = constructRay();
    color = castRay(ray);
}
