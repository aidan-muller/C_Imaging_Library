#include <stdio.h>
#include <stdlib.h>
#include "cil.h"

int main() {
	int *file_data;
	int* compressed_data;
	int *decompressed_data;
	size_t file_size, compressed_size, decompressed_size;
	int image_width, image_height, color_type, interlace;
	char *file_name = "png0.png";
	FILE *output_file;
	file_size = size_png(file_name);
	file_data = calloc(file_size, sizeof(int));
	read_png(file_name, file_data);
	get_png_details(file_data, file_size, &image_width, &image_height, &color_type, &interlace);
	printf("%d %d %d %d\n", image_width, image_height, color_type, interlace);
	compressed_size = get_compressed_png_data_size(file_data, file_size);
	compressed_data = calloc(compressed_size, sizeof(int));
	decompressed_size = get_decompressed_png_data_size(image_width, image_height, color_type);
	decompressed_data = calloc(compressed_size*compressed_size, sizeof(int));
	get_compressed_png_data(file_data, compressed_size, compressed_data);
	decompress_compressed_png_data(compressed_data, compressed_size, decompressed_size, decompressed_data);
	output_file = fopen("output.txt", "a+");
	for (int i = 0; i < decompressed_size; i++) {
		fprintf(output_file, "%d\n", decompressed_data[i]);
	}
	fclose(output_file);
	free(file_data);
	free(compressed_data);
	return 0;
}
