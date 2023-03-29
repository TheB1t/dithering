#include <iostream>
#include <stdio.h>
#include <math.h>

#pragma pack(push, 1)

struct BMP_HEADER {
	uint16_t type;			//2 bytes
	uint32_t size;			//4 bytes
	uint16_t reserved[2];	//2 x 2 = 4 bytes
	uint32_t offBits;		//4 bytes
};							//Total struct size 14 bytes

struct BMP_INFOHEADER {
	uint32_t size;			//4 bytes
	uint32_t width;			//4 bytes
	uint32_t height;		//4 bytes
	uint16_t planes;		//2 bytes
	uint16_t bitCount;		//2 bytes
	uint32_t compression;	//4 bytes
	uint32_t sizeImage;		//4 bytes
	uint32_t unused[25];	//4 x 25 = 100 bytes
};							//Total struct size 124 bytes (INFOHEADER V5)

struct RGB32 {
	uint8_t b;				//1 byte
	uint8_t g;				//1 byte
	uint8_t r;				//1 byte
	uint8_t unused;			//1 byte
};							//Total struct size 4 bytes

struct RGB24 {
	uint8_t b;				//1 byte
	uint8_t g;				//1 byte
	uint8_t r;				//1 byte
};							//Total struct size 3 bytes

#pragma pack(pop)

#define RGB RGB24

int divider = 42;
int core[3][5] = {
	{	0,	0,	0,	8,	4	},
	{	2,	4,	8,	4,	2	},
	{	1,	2,	4,	2,	1	}
};

int main() {
	FILE* image = fopen("image.bmp", "rb");
	BMP_HEADER head;
	BMP_INFOHEADER infohead;

	RGB* pixeldata;
	if (image) {
		std::cout << "Open success " << sizeof(BMP_HEADER) << std::endl;

		fread(&head, sizeof(BMP_HEADER), 1, image);
		fread(&infohead, sizeof(BMP_INFOHEADER), 1, image);

		std::cout << "Signature: "			<< std::hex << head.type			<< std::endl;
		std::cout << "Size in bytes: "		<< std::dec << head.size			<< std::endl;
		std::cout << "Data offset: "		<< std::hex << head.offBits			<< std::endl;

		std::cout << "INFO HEADER size: "	<< std::dec << infohead.size		<< std::endl;
		std::cout << "Width: "				<< std::dec << infohead.width		<< std::endl;
		std::cout << "Height: "				<< std::dec << infohead.height		<< std::endl;
		std::cout << "Planes: "				<< std::dec << infohead.planes		<< std::endl;
		std::cout << "Bit count: "			<< std::dec << infohead.bitCount	<< std::endl;
		std::cout << "Compression: "		<< std::dec << infohead.compression	<< std::endl;
		std::cout << "Size image: "			<< std::dec << infohead.sizeImage	<< std::endl;

		pixeldata = (RGB*)malloc(infohead.sizeImage);
		fread(pixeldata, sizeof(RGB), infohead.sizeImage / sizeof(RGB), image);

		fclose(image);
	}

	float error = 0.0f;

	for (int i = 0; i < infohead.height; i++) {
		for (int j = 0; j < infohead.width; j++) {
			RGB* data = &pixeldata[(i * infohead.width) + j];
			uint8_t S = (data->r + data->g + data->b) / 3;

			if (S < (0xFF / 2)) {
				error = (S - 0x00) / divider;
				S = 0x00;
			} else {
				error = (S - 0xFF) / divider;
				S = 0xFF;
			}

			*data = { S, S, S };

			for (int l = 0; l < 3; l++) {
				for (int c = 0; c < 5; c++) {
					if (core[l][c] > 0) {
						int x = j + (c - 3);
						int y = i + l;

						//std::cout << "X: " << x << " Y: " << y << std::endl;
						if (x < 0 || x > infohead.width) continue;
						if (y < 0 || y > infohead.height) continue;
						
						data = &pixeldata[(y * infohead.width) + x];
						S = (data->r + data->g + data->b) / 3;
						S += core[l][c] * error;
						*data = { S, S, S };
					}
				}
			}
		}
	}

	FILE* image2 = fopen("imageDithering.bmp", "wb");
	if (image2) {
		fwrite(&head, sizeof(BMP_HEADER), 1, image2);
		fwrite(&infohead, sizeof(BMP_INFOHEADER), 1, image2);
		fwrite(pixeldata, sizeof(RGB), infohead.sizeImage / sizeof(RGB), image2);
		fclose(image2);
	}
}
