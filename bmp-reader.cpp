#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#pragma pack(push,1)

struct BMPHeader {
	uint16_t file_type{ 0 };			// Type of the file, for BMP should be 0x4D42
	uint32_t file_size{ 0 };			// Size of the files in bytes
	uint16_t reserved_1{ 0 };			// Reserved
	uint16_t reserved_2{ 0 };			// Reserved
	uint32_t offset{ 0 };				// Offset for pixel data from beginning of file
};

struct DIBHeader {
	uint32_t size{ 0 };					// Size of DIB-header in bytes
	int32_t width{ 0 };					// width in pixels
	int32_t height{ 0 };				// height in pixels

	uint16_t planes{ 1 };				// number of color planes used
	uint16_t bit_count{ 0 };			// number of bits per pixel
	uint32_t compression{ 0 };			// compression
	uint32_t size_image{ 0 };			// size of the raw bitmap data (including padding)

	int32_t x_pixels_per_meter{ 0 };	// horizontal resolution of the image (pixel per meter, signed int)
	int32_t y_pixels_per_meter{ 0 };	// vertical resolution of the image (pixel per meter, signed int)
	uint32_t colors_used{ 0 };			// number of colors in the color palette    
	uint32_t colors_important{ 0 };		// number of important colors used
};

struct BMPColorHeader {
	uint32_t red_mask{ 0x00ff0000 };         // Bit mask for the red channel
	uint32_t green_mask{ 0x0000ff00 };       // Bit mask for the green channel
	uint32_t blue_mask{ 0x000000ff };        // Bit mask for the blue channel
	uint32_t alpha_mask{ 0xff000000 };       // Bit mask for the alpha channel
	uint32_t color_space_type{ 0x73524742 }; // Default "sRGB" (0x73524742)
	uint32_t unused[16]{ 0 };                // Unused data for sRGB color space
};

#pragma pack(pop)

struct BMP {

	BMPHeader file_header;
	DIBHeader dib_header;
	BMPColorHeader bmp_color_header;
	vector<uint8_t> data;

	BMP(const string filename) {
		read(filename);
	}

	void read(const string filename) {

		ifstream fin{ filename, ios_base::binary };

		if (fin) {
			fin.read((char*)&file_header, sizeof(file_header));

			if (file_header.file_type != 0x4D42) {
				throw runtime_error("Error! Not BMP file");
			}

			fin.read((char*)&dib_header, sizeof(dib_header));

			// Resize vector to total number of pixels
			data.resize(dib_header.width * dib_header.height);

			// Jump to pixel data location
			fin.seekg(file_header.offset, fin.beg);

			int bBit;
			uint32_t whiteMask;

			// Only for 24,32 bits
			switch (dib_header.bit_count) {
			case 24:
				bBit = 3;
				whiteMask = 0x00FFFFFF;
				break;
			case 32:
				bBit = 4;
				whiteMask = 0xFFFFFFFF;
				break;
			default:
				throw runtime_error("Error! Only 24 and 32 bits are supported!");
			}

			for (size_t i = 0; i < dib_header.width * dib_header.height; i++) {
				uint32_t pixel{0};
				fin.read((char*)&pixel, bBit);
				data[i] = pixel == whiteMask ? 0 : 1;
			}

		}
	}

	void print() {
		// Bitmap is usually stored bottom-up, left-right
		for (int i = dib_header.height-1; i > 0; --i) {
			for (int j = 0; j < dib_header.width; ++j) {
				char c = data[i*dib_header.width + j] == 0 ? ' ' : '#';
				cout << c;
			}
			cout << endl;
		}
	}

};


int main()
{
	string filename = "test_32bit.bmp";

	BMP image(filename);
	cout << image.file_header.file_size  << " bytes"s << endl;
	cout << image.dib_header.width << "x"s << image.dib_header.height << "px "s << image.dib_header.bit_count << "-bit"s << endl;
	image.print();

}

