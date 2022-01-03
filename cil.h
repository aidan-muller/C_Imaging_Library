#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

// Get size of png
size_t size_png(char* filename) {
	FILE *fp;
	size_t file_size = 0;
	fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp);
	return file_size;
}

// Get raw data of png
void read_png(char* filename, int* data) {
	FILE *fp;
	size_t file_size = 0;
	int c;
	fp = fopen(filename, "rb");
	int i = 0;
	while ((c = fgetc(fp)) != EOF) {
		data[i] = c;
		i += 1;
	}
	fclose(fp);
}

// Find chunk in png
int find_chunk_in_png(char* chunk, int* data, size_t file_size) {
	int found;
	for (int i = 0; i < file_size; i++) {
		found = 1;
		for (int j = 0; j < strlen(chunk)-1; j++) {
			if (data[i+j] != chunk[j]) {
				found = 0;
				break;
			}
		}
		if (found) {
			return i;
		}
	}
	return -1;
}

// Convert int array to one number (e.g.) 03 42 89 becomes 34289
int format_int(int* int_array, size_t size) {
	char return_value[4][3];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			return_value[i][j] = 0;
		}
	}
	char* final_value;
	int int_return;
	final_value = (char *) calloc(9, sizeof(char));
	for (int i = 0; i < size; i++) {
		char str[2];
		sprintf(str, "%x", int_array[i]);
		// If the number read from the data, and converted to hex, is 2 characters long, leave as is
		if (strlen(str) == 2) {
			strcpy(return_value[i], str);
		}
		// If it is one, add a 0 in front
		else {
			char nstr[3];
			sprintf(nstr, "0%d", atoi(str));
			strcpy(return_value[i], nstr);
		}
	}
	// Join together
	for (int i = 0; i < 4; i++) {
		sprintf(final_value, "%s%s", final_value, return_value[i]);
	}
	// Convert back to int
	int_return = (int)strtol(final_value, NULL, 16);
	free(final_value);
	return int_return;
}
// Get PNG details from IHDR
void get_png_details(int* data, size_t file_size, int* w, int* h, int* color_type, int* interlace_method) {
	int ihdr_position = find_chunk_in_png("IHDR", data, file_size);
	int* width = calloc(4, sizeof(int));
	int* height = calloc(4, sizeof(int));
	for (int i = 0; i < 4; i++) {
		width[i] = data[ihdr_position+4+i];
	}
	for (int i = 0; i < 4; i++) {
		height[i] = data[ihdr_position+8+i];
	}
	*color_type = data[ihdr_position+13];
	*interlace_method = data[ihdr_position+16];
	*w = format_int(width, 4);
	*h = format_int(height, 4);
	free(width);
	free(height);
}

// Get compressed PNG data size
size_t get_compressed_png_data_size(int* data, size_t file_size) {
	int idat = find_chunk_in_png("IDAT", data, file_size)+4;
	int iend = find_chunk_in_png("IEND", data, file_size);
	return iend-idat;
}

// Get compressed PNG data 
void get_compressed_png_data(int* data, size_t file_size, int* output) {
	int idat = find_chunk_in_png("IDAT", data, file_size)+4;
	int iend = find_chunk_in_png("IEND", data, file_size);
	for (int i = 0; i < file_size; i++) {
		output[i] = data[i+idat];
		// sprintf(output, "%s%c", output, data[i]);
	}
}

// Get size of decompressed data
size_t get_decompressed_png_data_size(int w, int h, int color_type) {
	switch (color_type) {
		case 2:
			return (w*h*3) + h;
			break;
		case 6:
			return (w*h*4) + h;
			break;
	}
	return -1;
}

// Decompress data (returns size of output)
void decompress_compressed_png_data(int* compressed_data, size_t compressed_size, size_t decompressed_size,int* output) {
	Bytef *istream = malloc(compressed_size);
	for (int i = 0; i < compressed_size; i++) {
		istream[i] = (Bytef) compressed_data[i];
	}
	ulong srcLen = decompressed_size*sizeof(Bytef *);
	ulong destLen = compressed_size;
	Bytef* ostream = malloc(srcLen);
	int des = uncompress((Bytef *)ostream, &srcLen, (Bytef *)istream, destLen);
	for (int i = 0; i < srcLen; i++) {
		output[i] = (int) ostream[i];
	}
	free(ostream);
	free(istream);
}

// Get pixel data
void get_png_pixels(int* data, size_t data_size, int w, int h, int color_type) {
	// Number of elements for the color type (e.g. RGB -> 3)
	int color_types[7] = {1, 0, 3, 1, 2, 0, 4};
	int pixel_data[h][w][color_types[color_type]];
	// Get size of each row
	int row_size = 1 + (w*color_types[color_type]);
	int filter_type;
	float divisibility;
	int which_pixel;
	int pixel_size = color_types[color_type];
	int row;
	for (int j = 0; j < data_size; j++) {
		// Get filter type of row
		filter_type = data[(int)floor(j/row_size)*row_size];
		// Determine if the data is the first of the row
		divisibility = ((float)j/(float)row_size);
		if (divisibility-floor(divisibility) == 0) {}
		else {
			// Get row of data;
			row = floor(divisibility);
			// Calculate which pixel it is
			which_pixel = floor((j-1)/pixel_size);
			// If the filter is none
			if (filter_type == 0) {
				// Return data as is
				pixel_data[row][which_pixel][(j-1)-(pixel_size*which_pixel)] = data[j];
			}
		}
	}
}
