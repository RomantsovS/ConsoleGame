#pragma hdrstop
#include <precompiled.h>


#include "Model_bmp.h"

void BMP::read(const std::string& fname) {
	idFileLocal file(fileSystem->OpenFileReadMemory(fname));

	if (file == nullptr) {
		return;
	}

	file->ReadBig(file_header);
	if (file_header.file_type != 0x4D42) {
		common->Error("Error! Unrecognized file format.");
	}
	file->ReadBig(bmp_info_header);

	// The BMPColorHeader is used only for transparent images
	if (bmp_info_header.bit_count == 32) {
		// Check if the file has bit mask color information
		if (bmp_info_header.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
			file->ReadBig(bmp_color_header);
			// Check if the pixel data is stored as BGRA and if the color space type is sRGB
			check_color_header(bmp_color_header);
		}
		else {
			common->Warning("Warning! The file %s does not seem to contain bit mask information\n", fname);
		}
	}

	// Adjust the header fields for output.
	// Some editors will put extra info in the image file, we only save the headers and the data.
	if (bmp_info_header.bit_count == 32) {
		bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
		file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
	}
	else {
		bmp_info_header.size = sizeof(BMPInfoHeader);
		file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
	}
	file_header.file_size = file_header.offset_data;

	if (bmp_info_header.height < 0) {
		common->Error("The program can treat only BMP images with the origin in the bottom left corner!");
	}

	data.resize(bmp_info_header.width * bmp_info_header.height * bmp_info_header.bit_count / 8);

	// Here we check if we need to take into account row padding
	if (bmp_info_header.width % 4 == 0) {
		for (size_t i = 0; i < data.size(); ++i) {
			file->ReadBig(*(data.data() + i));
		}
		file_header.file_size += data.size();
	}
	else {
		row_stride = bmp_info_header.width * bmp_info_header.bit_count / 8;
		const uint32_t new_stride = make_stride_aligned(4);
		std::vector<uint8_t> padding_row(new_stride - row_stride);

		for (int y = 0; y < bmp_info_header.height; ++y) {
			for (size_t i = 0; i < row_stride; ++i) {
				file->ReadBig(*(data.data() + row_stride * y + i));
			}
			
			for (size_t i = 0; i < padding_row.size(); ++i) {
				file->ReadBig(*(padding_row.data() + i));
			}
			//inp.read((char*)(data.data() + row_stride * y), row_stride);
			//inp.read((char*)padding_row.data(), padding_row.size());
		}
		file_header.file_size += data.size() + bmp_info_header.height * padding_row.size();
	}
}

// Check if the pixel data is stored as BGRA and if the color space type is sRGB
void BMP::check_color_header(BMPColorHeader & bmp_color_header) {
	BMPColorHeader expected_color_header;
	if (expected_color_header.red_mask != bmp_color_header.red_mask ||
		expected_color_header.blue_mask != bmp_color_header.blue_mask ||
		expected_color_header.green_mask != bmp_color_header.green_mask ||
		expected_color_header.alpha_mask != bmp_color_header.alpha_mask) {
		common->Error("Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
	}
	if (expected_color_header.color_space_type != bmp_color_header.color_space_type) {
		common->Error("Unexpected color space type! The program expects sRGB values");
	}
}

uint32_t BMP::make_stride_aligned(uint32_t align_stride) {
	uint32_t  new_stride = row_stride;
	while (new_stride % align_stride != 0) {
		new_stride++;
	}
	return new_stride;
}
