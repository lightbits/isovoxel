#ifndef IMAGE_H
#define IMAGE_H
#include "vec.h"
#include "types.h"

class Image
{
public:
	Image(uint32 width, uint32 height);
	~Image();

	void applyTonemap(float exposure = 0.15f, 
	                  float white = 20.0f, 
	                  float gamma = 1.0f / 2.2f);
	
	bool saveToFile(const char *filename) const;
	vec3 getPixel(uint32 x, uint32 y) const;
	void setPixel(uint32 x, uint32 y, const vec3 &pixel);
	void setLine(uint32 x, uint32 y0, uint32 y1, const vec3 &color);
	uint32 getWidth() const { return width; }
	uint32 getHeight() const { return height; }
private:
	uint32 width;
	uint32 height;
	vec3 *pixels;
};

#endif