
#include "idlib/precompiled.h"

#include "tr_local.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/*
===============
ActuallyLoadImage

Absolutely every image goes through this path
On exit, the idImage will have a valid OpenGL texture number that can be bound
===============
*/
void idImage::ActuallyLoadImage(bool fromBackEnd) {

	// if we don't have a rendering context yet, just return
	/*if (!R_IsInitialized()) {
		return;
	}*/

	// this is the ONLY place generatorFunction will ever be called
	if (generatorFunction) {
		generatorFunction(this);
		return;
	}	
	
	std::string full_path; 
	{
		idFileLocal file(fileSystem->OpenFileReadMemory(imgName));

		if (file == nullptr) {
			MakeDefault();
			return;
		}

		full_path = file->GetFullPath();
	}

	auto deleter = [](unsigned char* ptr) {
		stbi_image_free(ptr);
	};

	int img_width{}, img_height{}, img_nrChannels{};
	std::unique_ptr<unsigned char, decltype(deleter)> data_ptr(stbi_load(full_path.c_str(),
		&img_width, &img_height, &img_nrChannels, 0), deleter);
	if (!data_ptr) {
		common->Warning("Failed to load %s", imgName.c_str());
		MakeDefault();
		return;
	}

	ConvertImageDataToModelPixels(data_ptr.get(), img_height, img_width, img_nrChannels, pixels);

	height = img_height;
	width = img_width;
}

bool idImage::ConvertImageDataToModelPixels(unsigned char* data, int img_height, int img_width, int img_nrChannels,
	std::vector<ModelPixel>& pixels) {
	const char symbol{ '\xDB' };
	int col{};

	idassert(img_nrChannels >= 3);

	for (int j = 0; j < img_height; ++j) {
		for (int i = 0; i < img_width; ++i) {
			int cur_pixel = (j * img_width + i) * img_nrChannels;

			if (data[cur_pixel + 0] > 200 && data[cur_pixel + 1] > 200 &&
				data[cur_pixel + 2] > 200) {
				col = colorWhite;
			}
			else if (data[cur_pixel + 1] > 100 && data[cur_pixel + 2] > 100 && data[cur_pixel + 0] < 100) {
				col = data[cur_pixel + 0] > 75 ? colorLightCyan : colorCyan;
			}
			else if (data[cur_pixel + 0] > 200 && data[cur_pixel + 2] > 200) {
				col = data[cur_pixel + 1] > 75 ? colorLightMagenta : colorMagenta;
			}
			else if (data[cur_pixel + 0] > 50 && data[cur_pixel + 0] < 200 && data[cur_pixel + 1] > 50 && data[cur_pixel + 1] < 200
				&& data[cur_pixel + 2] < 50) {
				col = colorBrown;
			}
			else if (data[cur_pixel + 0] > 100 && data[cur_pixel + 1] > 100 && data[cur_pixel + 2] < 100) {
				col = colorYellow;
			}
			else if (data[cur_pixel + 2] > data[cur_pixel + 0] + data[cur_pixel + 1]) {
				col = (data[cur_pixel + 0] + data[cur_pixel + 1] + data[cur_pixel + 2]) > 300 ? colorLightBlue : colorBlue;
			}
			else if (data[cur_pixel + 1] > data[cur_pixel + 0] + data[cur_pixel + 2]) {
				col = (data[cur_pixel + 0] + data[cur_pixel + 1] + data[cur_pixel + 2]) > 200 ? colorLightGreen : colorGreen;
			}
			else if (data[cur_pixel + 0] > data[cur_pixel + 1] + data[cur_pixel + 2]) {
				col = (data[cur_pixel + 0] + data[cur_pixel + 1] + data[cur_pixel + 2]) > 300 ? colorLightRed : colorRed;
			}
			else if (data[cur_pixel + 0] < 50 && data[cur_pixel + 1] < 50 &&
				data[cur_pixel + 2] < 50) {
				col = colorBlack;
			}
			else if (data[cur_pixel + 0] + data[cur_pixel + 1] + data[cur_pixel + 2] > 300) {
				col = colorLightGray;
			}
			else
				col = colorDarkGray;

			pixels.emplace_back(Vector2(i, j), Screen::Pixel(symbol, col));
		}
	}

	return true;
}