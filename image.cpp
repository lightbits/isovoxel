#include "image.h"
#include "types.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

float toLuminance(float R, float G, float B) 
{
	return 0.2126f * R + 0.7152f * G + 0.0722 * B; 
}

// Computes the (log) average of the luminance of the image by eq. 1 in [Reinhard]
float logAverageLuminance(const vec3 *pixels, uint32 width, uint32 height)
{
	const float delta = 0.001f; // Small value to avoid singularity if black pixels are present
	float sum = 0.0f;

	// Sum up the log of the luminance value of each pixel
	for(uint32 y = 0; y < height; ++y)
	{
		for(uint32 x = 0; x < width; ++x)
		{
			vec3 pixel = pixels[y * width + x];
			sum += logf(delta + toLuminance(pixel.x, pixel.y, pixel.z));
		}
	}

	int n = width * height; // Number of pixels in image
	return expf(sum / float(n));
}

Image::Image(uint32 width_, uint32 height_) : 
	width(width_), height(height_)
{
	pixels = new vec3[width * height];
	for (int i = 0; i < width * height; ++i)
		pixels[i] = vec3(0.0f, 0.0f, 0.0f);
}

Image::~Image()
{
	delete[] pixels;
}

/* 
	See http://imdoingitwrong.wordpress.com/tag/hdr/
	Applies the tonemap operator defined in (Eq. 4) of http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf
	(the Reinhard operator).
	@param exposure: Key (image specific)
	@param white: The smallest luminance that will be mapped to pure white
	@param gamma: Gamma correction factor (usually 0.45)
*/
void Image::applyTonemap(float exposure, float white, float gamma)
{
	float lumAvg = logAverageLuminance(pixels, width, height);

	for(uint32 y = 0; y < height; ++y)
	{
		for(uint32 x = 0; x < width; ++x)
		{
			vec3 pixel = getPixel(x, y);
			float R = pixel.x;
			float G = pixel.y;
			float B = pixel.z;

			// Calculate pixel luminance
			float Lp = toLuminance(R, G, B);

			// Scale the pixel luminance to a middle gray zone
			float L = Lp * exposure / lumAvg;

			// Apply modified tonemapping operator and compress luminance to displayable range
			float Ld = L * (1.0f + (L / (white * white))) / (1.0f + L);

			float scale = Ld / Lp;

			// Scale and clamp all colors by the relative luminance gain
			R = std::min(R * scale, 1.0f);
			G = std::min(G * scale, 1.0f);
			B = std::min(B * scale, 1.0f);
			R = pow(R, gamma);
			G = pow(G, gamma);
			B = pow(B, gamma);
			setPixel(x, y, vec3(R, G, B));
		}
	}
}

bool Image::saveToFile(const char *filename) const
{
	// Convert floating point rgb data to uint8 buffer
	const uint32 stride = width * 3;
	uint8 *pbuffer = new uint8[width * height * 3];

	// Transfer pixel data
	const uint32 numpixels = width * height;
	for(uint32 i = 0; i < numpixels; ++i)
	{
		vec3 color = pixels[i];

		// Clamp to valid range
		uint8 r = uint8(std::min(int(color.x * 255.0f), 255));
		uint8 g = uint8(std::min(int(color.y * 255.0f), 255));
		uint8 b = uint8(std::min(int(color.z * 255.0f), 255));

		// Write to buffer
		uint32 byteOffset = i * 3;
		pbuffer[byteOffset + 0] = r;
		pbuffer[byteOffset + 1] = g;
		pbuffer[byteOffset + 2] = b;
	}

	int status = stbi_write_png(filename, width, height, 3, pbuffer, stride);
	delete[] pbuffer;
	return status != 0;
}

vec3 Image::getPixel(uint32 x, uint32 y) const
{
	return pixels[y * width + x];
}

void Image::setPixel(uint32 x, uint32 y, const vec3 &pixel)
{
	// Subtract y to flip image so that
	// (0, 0) is the bottom-left, and (width, height) is top-right
	pixels[(height - 1 - y) * width + x] = pixel;
}

void Image::setLine(uint32 x, uint32 y0, uint32 y1, const vec3 &color)
{
	y0 = max(y0, 0);
	y1 = min(y1, height - 1);
	for (int y = y0; y <= y1; ++y)
		setPixel(x, y, color);
}