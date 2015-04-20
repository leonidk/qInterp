#pragma once
#include <memory>
#include <cmath>

namespace img {
	
	template <typename T, int C>
	struct Image {
		std::shared_ptr<T> data;
		int width, height;
		Image() : data(nullptr),width(0),height(0) {}
		Image(int width, int height) : data(new T[width*height*C], arr_d<T>()),width(width),height(height) {}
		Image(int width, int height, T* d) : data(d, null_d<T>()), width(width), height(height)  {}

		T* const ptr() { return (T*)data.get(); }

		template< typename T >
		struct null_d { void operator ()(T const * p)	{ } };
		template< typename T >
		struct arr_d { void operator ()(T const * p)	{ delete[] p; } };
	};

	template<typename T> using Img = Image<T, 1>;
}



inline void generatePoints(img::Img<uint16_t> input, const float fx, const float fy, const float px, const float py, img::Image<float,3> output) {
	auto halfX = px;
	auto halfY = py;
	auto cX = 1.0f / fx;
	auto cY = 1.0f / fy;
	auto depth = input.ptr();
	auto points = output.ptr();
	for (int i = 0; i < input.height; i++) {
		for (int j = 0; j < input.width; j++) {
			const auto z = depth[(i * input.width + j)];
			points[3 * (i * input.width + j)] = (j - halfX) * z * cX;
			points[3 * (i * input.width + j) + 1] = (i - halfY) * z * cY;
			points[3 * (i * input.width + j) + 2] = z;
		}
	}
}



template <typename T>
inline T square(const T a) {
	return a * a;
}

inline float sqrNorm(const float *a, const float *b) {
	return square(a[0] - b[0]) + square(a[1] - b[1]) + square(a[2] - b[2]);
}

inline void cross(const float *a, const float *b, float *c) {
	c[0] = (a[1] * b[2]) - (a[2] * b[1]);
	c[1] = (a[2] * b[0]) - (a[0] * b[2]);
	c[2] = (a[0] * b[1]) - (a[1] * b[0]);
}
inline float dot(const float *a, const float *b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
inline void normalize(float *a) {
	const auto norm = sqrt(dot(a, a));
	a[0] /= norm;
	a[1] /= norm;
	a[2] /= norm;
}

inline img::Image<float, 3> generatePoints(img::Img<uint16_t> input, const float fx, const float fy, const float px, const float py) {
	img::Image<float, 3> output(input.width, input.height);
	generatePoints(input, fx, fy, px, py,output);
	return output;
}

template <int size>
inline void generateNormals_fromPoints(img::Image<float, 3> input, img::Image<float, 3> output) {
	auto height = input.height;
	auto width = input.width;
	auto points = input.ptr();
	auto normals = output.ptr();
	memset(normals,0, width*height*sizeof(float) * 3);
	for (int i = size; i < height - size; i++) {
		for (int j = size; j < width - size; j++) {
			if (!points[3 * (i * width + j) + 2])
				continue;
			const float *pc = &points[3 * (i * width + j)];

			float outNorm[3] = { 0, 0, 0 };
			int count = 0;
			if (points[3 * (i * width + j + size) + 2] && points[3 * ((i + size) * width + j) + 2]) {
				const float *px = &points[3 * (i * width + j + size)];
				const float *py = &points[3 * ((i + size) * width + j)];

				float v1[] = { px[0] - pc[0], px[1] - pc[1], px[2] - pc[2] };
				float v2[] = { py[0] - pc[0], py[1] - pc[1], py[2] - pc[2] };

				float v3[3];
				cross(v1, v2, v3);
				normalize(v3);
				outNorm[0] += v3[0];
				outNorm[1] += v3[1];
				outNorm[2] += v3[2];
				count++;
			}
			if (points[3 * (i * width + j - size) + 2] && points[3 * ((i + size) * width + j) + 2]) {
				const float *px = &points[3 * (i * width + j - size)];
				const float *py = &points[3 * ((i + size) * width + j)];

				float v1[] = { pc[0] - px[0], pc[1] - px[1], pc[2] - px[2] };
				float v2[] = { py[0] - pc[0], py[1] - pc[1], py[2] - pc[2] };

				float v3[3];
				cross(v1, v2, v3);
				normalize(v3);
				outNorm[0] += v3[0];
				outNorm[1] += v3[1];
				outNorm[2] += v3[2];
				count++;
			}
			if (points[3 * (i * width + j + size) + 2] && points[3 * ((i - size) * width + j) + 2]) {
				const float *px = &points[3 * (i * width + j + size)];
				const float *py = &points[3 * ((i - size) * width + j)];

				float v1[] = { px[0] - pc[0], px[1] - pc[1], px[2] - pc[2] };
				float v2[] = { pc[0] - py[0], pc[1] - py[1], pc[2] - py[2] };

				float v3[3];
				cross(v1, v2, v3);
				normalize(v3);
				outNorm[0] += v3[0];
				outNorm[1] += v3[1];
				outNorm[2] += v3[2];
				count++;
			}
			if (points[3 * (i * width + j - size) + 2] && points[3 * ((i - size) * width + j) + 2]) {
				const float *px = &points[3 * (i * width + j - size)];
				const float *py = &points[3 * ((i - size) * width + j)];

				float v1[] = { pc[0] - px[0], pc[1] - px[1], pc[2] - px[2] };
				float v2[] = { pc[0] - py[0], pc[1] - py[1], pc[2] - py[2] };

				float v3[3];
				cross(v1, v2, v3);
				normalize(v3);
				outNorm[0] += v3[0];
				outNorm[1] += v3[1];
				outNorm[2] += v3[2];
				count++;
			}
			if (count) {
				float v3[3] = { outNorm[0] / count, outNorm[1] / count, outNorm[2] / count };
				normalize(v3);

				normals[3 * (i * width + j)] = v3[0];
				normals[3 * (i * width + j) + 1] = v3[1];
				normals[3 * (i * width + j) + 2] = v3[2];

				//double d = -v3[0] * pc[0] - v3[1] * pc[1] - v3[2] * pc[2];
			}
		}
	}
}



template <int size>
inline img::Image<float, 3> generateNormals_fromPoints(img::Image<float, 3> input) {
	img::Image<float, 3> output(input.width, input.height);
	generateNormals_fromPoints<size>(input, output);
	return output;
}