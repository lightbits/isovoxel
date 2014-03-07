#include <iostream>
#include "image.h"
#include "vec.h"
#include "types.h"

const float PI2 = 1.57079632679f;
const float PI = 3.14159265359f;

vec3 c_eye = vec3(0.0f, 4.0f, 0.0f);
vec3 c_fwd = vec3(0.0f, -1.0f, 0.0f);
vec3 c_rht = vec3(1.0f, 0.0f, 0.0f);
vec3 c_up  = vec3(0.0f, 0.0f, 1.0f);

vec3 c_sky_color = vec3(0.0f);
float scale_x = 0.5f;
float scale_y = 0.5f;
float hmap_height = 0.8f;
int hmap_res_x = 32;
int hmap_res_y = 32;

vec3 toSpherical(float theta, float phi, float rho)
{
	float r = rho * cos(phi);
	return vec3(r * sin(theta), rho * sin(phi), r * cos(theta));
}

void setViewpoint(float theta, float phi, float rho, const vec3 &center)
{
	c_eye = center;
	c_fwd = normalize(toSpherical(theta, phi, rho));
	c_rht = normalize(toSpherical(theta + PI2, 0.0f, rho));
	c_up = normalize(cross(c_fwd, c_rht));
}

vec3 project(float u, float v)
{
	// generate a ray orthographic to the image plane
	vec3 ro = c_eye + c_rht * u / scale_x + c_up * v / scale_y;
	vec3 rd = c_fwd;

	// intersect with [0, 1] x [0, 1] xz-plane
	float t = -ro.y / rd.y;
	return ro + rd * t;
}

vec2 unproject(const vec3 &world)
{
	return vec2(0.0f, 0.0f);
}

// x and y in range [0, 1]
float sampleHeightmap(float x, float y)
{
	int xi = floor(0.25f * x * hmap_res_x);
	int yi = floor(0.25f * y * hmap_res_y);

	// float heights[6][6] = {
	// 	0.02f, 0.02f, 0.02f, 0.02f, 0.2f, 0.2f,
	// 	0.02f, 0.02f, 0.2f, 0.2f, 0.2f, 0.2f,
	// 	0.02f, 0.02f, 1.0f, 0.2f, 0.2f, 0.2f,
	// 	0.02f, 0.02f, 0.2f, 0.2f, 0.2f, 0.2f,
	// 	0.02f, 0.02f, 0.2f, 0.2f, 0.2f, 0.2f,
	// 	0.02f, 0.02f, 0.2f, 0.2f, 0.2f, 0.2f,
	// };

	// return heights[xi][yi];

	float a = 2.0f * (xi / float(hmap_res_x)) - 1.0f;
	float b = 2.0f * (yi / float(hmap_res_y)) - 1.0f;
	return exp(-(a * a + b * b));
}

void fillColumn(Image *image, int x, int y, int width, int height)
{
	float u = (x / float(width)) * 2.0f - 1.0f;
	float v = (y / float(height)) * 2.0f - 1.0f;

	// project image plane position onto xz-plane
	vec3 p = project(u, v);
	if (p.x < 0.0f || p.x > 4.0f ||
	    p.z < 0.0f || p.z > 4.0f)
	{
	    image->setPixel(x, y, c_sky_color);
		return;
	}

	float h = sampleHeightmap(p.x, p.z) * hmap_height;

	// project top of height sample (in world coordinates)
	// back to the uv-plane
	// (bad maths incoming)
	float cost = -c_fwd.y / length(c_fwd);
	float dx = h * cost;
	float da = sqrt(h * h - dx * dx);
	float v_end = v + da * scale_y;
	int y_end = int((v_end + 1.0f) * 0.5f * height);

	for (int yp = y; yp < y_end; ++yp)
	{
		if (yp >= 0 && yp < height)
			image->setPixel(x, yp, min(vec3(h), vec3(1.0f)));
	}
}

vec3 computeFragment(float u, float v)
{
	// project image plane position onto xz-plane
	vec3 p = project(u, v);
	if (p.x < 0.0f || p.x > 1.0f ||
	    p.z < 0.0f || p.z > 1.0f)
	    return c_sky_color;

	float h = sampleHeightmap(p.x, p.z) * hmap_height;

	// project top of height sample (in world coordinates)
	// back to the uv-plane
	// (bad maths incoming)
	float cost = -c_fwd.y / length(c_fwd);
	float dx = h * cost;
	float da = sqrt(h * h - dx * dx);
	float vproj = v + da * scale_y;

	return vec3(h);
}

void render(Image *image)
{
	const int width = image->getWidth();
	const int height = image->getHeight();
	for (int x = 0; x < width; ++x)
	{
		for (int y = height - 1; y >= 0; --y)
		{
			fillColumn(image, x, y, width, height);
		}
	}
}

int main(int argc, char **argv)
{
	uint32 width = 500;
	uint32 height = 500;
	Image image(width, height);

	setViewpoint(-PI/4.0f, -PI/6.0f, 1.0f, vec3(2.4f, 1.4f, -1.0f));
	render(&image);

	image.saveToFile("result.png");
	std::cout << "Result saved as result.png" << std::endl;

	return 0;
}