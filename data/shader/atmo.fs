// Copyright (C) 2016  www.scratchapixel.com
//               2017  En Shih
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

uniform vec2 WindowSize;
uniform float sunAngle;
uniform float fov;

#ifndef M_PI
#define M_PI (3.14159265358979323846f)
#endif

struct Camera
{
	vec3 pos;
	vec3 dir;
	vec3 right;
	vec3 up;
	float fov;
};

vec3 createCameraRay(Camera camera)
{
	vec2 uv = gl_FragCoord.xy / WindowSize;
	float angle = tan(camera.fov * 0.5f);
	return normalize((uv.x * 2.0f - 1.0f) * camera.right * (WindowSize.x / WindowSize.y) * angle + (1.0f - uv.y * 2.0f) * camera.up * angle + camera.dir);
}

Camera lookAt (vec3 camPos, vec3 lookAt, vec3 upVec)
{
	vec3 diffBtw = lookAt - camPos;
	Camera camera;
	camera.pos = camPos;
	camera.right = normalize(cross(diffBtw, upVec));
	camera.dir = normalize(diffBtw);
	camera.up = normalize(upVec);
	return camera;
}

vec3 raySphereIntersect(const vec3 orig, const vec3 dir, const float radius)
{
	float t0, t1;

    // They ray dir is normalized so A = 1
    float a = dot(dir, dir);
    float b = 2 * dot(dir, orig);
    float c = dot(orig, orig) - radius * radius;

    if (b == 0)
	{
        // Handle special case where the the two vector ray.dir and V are perpendicular
        // with V = ray.orig - sphere.centre
        if (a == 0) return vec3(false);
        t0 = 0;
		t1 = sqrt(-c / a);
    }
	else
	{
		float D = b*b - 4*a*c;
		if (D < 0) return vec3(false);

		float q = (b < 0.f) ? -0.5f * (b - sqrt(D)) : -0.5f * (b + sqrt(D));
		t0 = q / a;
		t1 = c / q;
	}

	return (t0 < t1) ? vec3(true, t0, t1) : vec3(true, t1, t0);
}

// [comment]
// The atmosphere class. Stores data about the planetory body (its radius), the atmosphere itself
// (thickness) and things such as the Mie and Rayleigh coefficients, the sun direction, etc.
// [/comment]
struct Atmosphere
{
    vec3 sunDirection;     // The sun direction (normalized)
    float earthRadius;      // In the paper this is usually Rg or Re (radius ground, eart)
    float atmosphereRadius; // In the paper this is usually R or Ra (radius atmosphere)
    float Hr;               // Thickness of the atmosphere if density was uniform (Hr)
    float Hm;               // Same as above but for Mie scattering (Hm)

    vec3 betaR;
    vec3 betaM;
};

vec3 computeIncidentLight(const vec3 orig, const vec3 dir, float tmin, float tmax, Atmosphere atmosphere)
{
	vec3 intersect = raySphereIntersect(orig, dir, atmosphere.atmosphereRadius);
	float t0 = intersect.y, t1 = intersect.z;
    if (!bool(intersect.x) || t1 < 0)
		return vec3(0);

    if (t0 > tmin && t0 > 0) tmin = t0;
    if (t1 < tmax) tmax = t1;
    int numSamples = 16;
    int numSamplesLight = 8;
    float segmentLength = (tmax - tmin) / numSamples;
    float tCurrent = tmin;
    vec3 sumR = vec3(0), sumM = vec3(0); // mie and rayleigh contribution
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dot(dir, atmosphere.sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
    float phaseR = 3.f / (16.f * M_PI) * (1 + mu * mu);
    float g = 0.76f;
    float phaseM = 3.f / (8.f * M_PI) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));
    for (int i = 0; i < numSamples; ++i)
	{
    	vec3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir;
        float height = length(samplePosition) - atmosphere.earthRadius;
        // compute optical depth for light
        float hr = exp(-height / atmosphere.Hr) * segmentLength;
        float hm = exp(-height / atmosphere.Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
		vec3 intersect = raySphereIntersect(samplePosition, atmosphere.sunDirection, atmosphere.atmosphereRadius);
		t0Light = intersect.y; t1Light = intersect.z;
        float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        int j;
        for (j = 0; j < numSamplesLight; ++j)
		{
            vec3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * atmosphere.sunDirection;
            float heightLight = length(samplePositionLight) - atmosphere.earthRadius;
            if (heightLight < 0) break;
            opticalDepthLightR += exp(-heightLight / atmosphere.Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / atmosphere.Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight)
		{
            vec3 tau = atmosphere.betaR * (opticalDepthR + opticalDepthLightR) + atmosphere.betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
            vec3 attenuation = vec3(exp(-tau.x), exp(-tau.y), exp(-tau.z));
            sumR += attenuation * hr;
            sumM += attenuation * hm;
        }
        tCurrent += segmentLength;
    }

    // [comment]
    // We use a magic number here for the intensity of the sun (20). We will make it more
    // scientific in a future revision of this lesson/code
    // [/comment]
    return (sumR * atmosphere.betaR * phaseR + sumM * atmosphere.betaM * phaseM) * 50;
}

Atmosphere atmo;
Camera camera;

void init()
{
	atmo.sunDirection = vec3(0, 1, 0);
	atmo.earthRadius = 6360e3;
	atmo.atmosphereRadius = 6420e3;
	atmo.Hr = 7994;
	atmo.Hm = 1200;
	atmo.betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	atmo.betaM = vec3(21e-6f);
}

vec3 color = vec3(0);

void renderFisheye()
{
	float y = 2.f * (gl_FragCoord.y + 0.5f) / float(WindowSize.y - 1) - 1.f;
	float x = 2.f * (gl_FragCoord.x + 0.5f) / float(WindowSize.x - 1) - 1.f;
	float z2 = x * x + y * y;
	if (z2 <= 1)
	{
		float phi = atan(y, x);
		float theta = acos(1 - z2);
		vec3 dir = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
		// 1 meter above sea level
		float tmax = 1e20;
		color = computeIncidentLight(vec3(0, atmo.earthRadius + 1, 0), dir, 0, tmax, atmo);
	}
}

void renderSkydome()
{
    int numPixelSamples = 3;

	for (int m = 0; m < numPixelSamples; ++m)
	{
		for (int n = 0; n < numPixelSamples; ++n)
		{
			//float random = 0.5; // TODO: implement random generator between [0, 1]
			//float rayx = (2 * (gl_FragCoord.x + (m + random) / numPixelSamples) / WindowSize.x - 1) * aspectRatio * angle;
			//float rayy = (2 * (gl_FragCoord.y + (n + random) / numPixelSamples) / WindowSize.y - 1) * angle;
			vec3 dir = createCameraRay(camera);

			// [comment]
			// Does the ray intersect the planetary body? (the intersection test is against the Earth here
			// not against the atmosphere). If the ray intersects the Earth body and that the intersection
			// is ahead of us, then the ray intersects the planet in 2 points, t0 and t1. But we
			// only want to comupute the atmosphere between t=0 and t=t0 (where the ray hits
			// the Earth first). If the viewing ray doesn't hit the Earth, or course the ray
			// is then bounded to the range [0:INF]. In the method computeIncidentLight() we then
			// compute where this primary ray intersects the atmosphere and we limit the max t range
			// of the ray to the point where it leaves the atmosphere.
			// [/comment]
			float t0, t1;
			float tMax = 1e20; // Infinity
			vec3 intersect = raySphereIntersect(camera.pos, dir, atmo.earthRadius);
			t0 = intersect.y; t1 = intersect.z;
			if (bool(intersect.x) && t1 > 0)
			 	tMax = max(0.f, t0);
			// [comment]
			// The *viewing or camera ray* is bounded to the range [0:tMax]
			// [/comment]
			color += computeIncidentLight(camera.pos, dir, 0, tMax, atmo);
		}
	}
	color *= 1.f / (numPixelSamples * numPixelSamples);
}

void main()
{
	init();

	atmo.sunDirection = vec3(0, cos(sunAngle), -sin(sunAngle));

	vec3 pos = vec3(0, atmo.earthRadius + 1000, 0);
	vec3 up = vec3(0, -1 ,0);
	camera = lookAt(pos, vec3(0, atmo.earthRadius + 1000, -1), up);
	camera.fov = fov;

	renderSkydome();
	//renderFisheye();

	color.x = color.x < 1.413f ? pow(color.x * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-color.x);
	color.y = color.y < 1.413f ? pow(color.y * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-color.y);
	color.z = color.z < 1.413f ? pow(color.z * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-color.z);

	gl_FragColor = vec4(color, 1);
}
