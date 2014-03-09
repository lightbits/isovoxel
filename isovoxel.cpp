#include <iostream>
#include "image.h"
#include "vec.h"
#include "types.h"
#include "noise.h"

const float PI2 = 1.57079632679f;
const float PI  = 3.14159265359f;

vec3 c_eye = vec3(0.0f, 4.0f, 0.0f);
vec3 c_fwd = vec3(0.0f, -1.0f, 0.0f);
vec3 c_rht = vec3(1.0f, 0.0f, 0.0f);
vec3 c_up  = vec3(0.0f, 0.0f, 1.0f);

vec3 c_sky_color = vec3(0.0f);
float scale_x = 0.8f;
float scale_y = 0.8f;
float hmap_height = 0.2f;
int hmap_res_x = 8;
int hmap_res_y = 8;

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

// Projects the point (u, v) from the image plane onto
// the y = 0 plane
vec3 project(float u, float v)
{
	// generate a ray orthographic to the image plane
	vec3 ro = c_eye + c_rht * u / scale_x + c_up * v / scale_y;
	vec3 rd = c_fwd;

	// intersect with y = 0 plane
	float t = -ro.y / rd.y;
	return ro + rd * t;
}

// Projects the column of height h onto the image plane,
// with start-coordinates (u0, v0). Returns the v component.
float unproject(float u0, float v0, float h)
{
	// Kinda sketchy math here
	float cost = -c_fwd.y / length(c_fwd);
	float dx = h * cost;
	float da = sqrt(h * h - dx * dx);
	return v0 + da * scale_y;
}

// x and y in range [0, 1]
float sampleHeightmap(float x, float y)
{
	float scale = 12.0f;
	float xf = floor(scale * x * hmap_res_x) / hmap_res_x;
	float yf = floor(scale * y * hmap_res_y) / hmap_res_y;
	float s = fBm(xf, yf, 2.33f, 0.43f, 5, 16123);

	return s * 0.5f + 0.5f;
}

float sampleHeightmap(float x, float y, Image *heightmap)
{
	float w = float(heightmap->getWidth());
	float h = float(heightmap->getHeight());
	uint32 xi = uint32(floor(x * w / 3.0f));
	uint32 yi = uint32(floor(y * h / 3.0f));

	xi = xi % heightmap->getWidth();
	yi = yi % heightmap->getHeight();
	
	vec3 pixel = heightmap->getPixel(xi, yi);
	return pixel.x;
}

void fillColumn(Image *image, Image *heightmap, int x, int y, int width, int height)
{
	float u = (x / float(width)) * 2.0f - 1.0f;
	float v = (y / float(height)) * 2.0f - 1.0f;

	// project image plane position onto xz-plane
	vec3 p = project(u, v);
	if (p.x < -1.0f || p.x > 1.0f ||
	    p.z < -1.0f || p.z > 1.0f)
	{
	    image->setPixel(x, y, c_sky_color);
		return;
	}

	// transform to [0, 1] range
	float sample_x = (p.x * 0.5f + 0.5f);
	float sample_y = (p.z * 0.5f + 0.5f);
	float h = sampleHeightmap(sample_x, sample_y, heightmap) * hmap_height;

	// transform column back to image plane coords
	float v1 = unproject(u, v, h);

	// the top of the column in pixel coords
	int y_end = int((v1 + 1.0f) * 0.5f * height);

	// approximate normal
	float eps = 0.1f;
	float dhx = (sampleHeightmap(sample_x + eps, sample_y, heightmap) -
				 sampleHeightmap(sample_x - eps, sample_y, heightmap)) * hmap_height;
	float dhy = eps;
	float dhz = (sampleHeightmap(sample_x, sample_y + eps, heightmap) -
				 sampleHeightmap(sample_x, sample_y - eps, heightmap)) * hmap_height;
	vec3 normal = normalize(vec3(dhx, dhy, dhz));

	static const vec3 light_pos = vec3(0.0f, 0.8f, 0.0f);
	static const vec3 light_col = vec3(0.84f, 0.6f, 0.5f);

	vec3 to_light = light_pos - vec3(p.x, h, p.z);
	float light_dist = length(to_light);
	vec3 light_dir = to_light * (1.0f / light_dist);
	float light_intensity = max(dot(light_dir, normal), 0.0f);
	light_intensity *= 4.0f / (light_dist * light_dist);
	vec3 color = light_col * light_intensity * h / hmap_height;

	// convert height to color and draw the height column
	// vec3 color = vec3(h) * 1.0f / hmap_height;
	// color = min(color, vec3(1.0f));


	image->setLine(x, y, y_end, color);
}

void render(Image *image, Image *heightmap)
{
	const int width = image->getWidth();
	const int height = image->getHeight();
	for (int x = 0; x < width; ++x)
	{
		for (int y = height - 1; y >= 0; --y)
		{
			fillColumn(image, heightmap, x, y, width, height);
		}
	}
}

#include <sstream>

int main(int argc, char **argv)
{
	uint32 width = 500;
	uint32 height = 500;
	Image image(width, height);

	Image heightmap;
	heightmap.loadFromFile("img/heightmap0.png");

	scale_x = atof(argv[1]);
	scale_y = atof(argv[2]);

	setViewpoint(-PI/4.0f, -PI/6.0f, 1.0f, vec3(3.0f, 2.3f, -3.0f));
	render(&image, &heightmap);
	image.saveToFile("img/result.png");
	std::cout << "Result saved as result.png" << std::endl;

	return 0;
}