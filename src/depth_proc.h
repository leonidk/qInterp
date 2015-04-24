#pragma once
#include "image.h"

#include <cmath>

inline void generatePoints(img::Img<uint16_t> input, const float fx, const float fy, const float px, const float py, img::Image<float, 3> output) {
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

template <int scaleFactor, int ITERNUM, bool doL1=true>
inline void generateDequant(img::Img<uint16_t> input_output)
{
	auto p = input_output.ptr();
	auto w = input_output.width;
	auto h = input_output.height;

	img::Img<uint8_t> edges(w,h);
	img::Img<uint32_t> shift(w, h);
	auto s = shift.ptr();
	memset(s, 0, w*h * 4);

	memset(edges.data.get(), 0, w*h);
	auto eP = edges.ptr();

	img::Image<uint32_t, 2> distance(w, h);
	img::Image<uint32_t, 2> minDistance(w, h);
	auto dP = distance.ptr();
	auto mdP = minDistance.ptr();
	for (int i = 0; i < 2 * distance.width*distance.height; i++) {
		dP[i] =  INT_MAX;
	}
	for (int i = 0; i < 2 * minDistance.width*minDistance.height; i++) {
		mdP[i] = 0;
	}
	for (int x = 0; x < w; x++) {
		auto y = 0;
		eP[y*w + x] = 3;
		dP[2 * (y*w + x) + 0] = 1;
		mdP[2 * (y*w + x) + 0] = 1;
		dP[2 * (y*w + x) + 1] = 1;
		mdP[2 * (y*w + x) + 1] = 1;
	}
	for (int y = 1; y < h - 1; y++) {
		int x = 0;
		eP[y*w + x] = 3;
		dP[2 * (y*w + x) + 0] = 1;
		mdP[2 * (y*w + x) + 0] = 1;
		dP[2 * (y*w + x) + 1] = 1;
		mdP[2 * (y*w + x) + 1] = 1;
		for (int x = 1; x < w - 1; x++) {
			int d = p[y*w + x];
			int dn = p[(y - 1)*w + x];
			int ds = p[(y + 1)*w + x];
			int dw = p[y*w + (x - 1)];
			int de = p[y*w + (x + 1)];

			if (!d) {
				eP[y*w + x] |= 1;
				dP[2 * (y*w + x) + 0] = 1;
				eP[y*w + x] |= 2;
				dP[2 * (y*w + x) + 1] = 1;
			}
			if ((dw - d) >= 1 || (de - d) >= 1 || (dn - d) >= 1 || (ds - d) >= 1) {
				eP[y*w + x] |= 1;
				dP[2 * (y*w + x) + 0] = 1;
				mdP[2 * (y*w + x) + 0] = 1;
			}
			if ((dw - d) <= -1 || (de - d) <= -1 || (dn - d) <= -1 || (ds - d) <= -1) {
				eP[y*w + x] |= 2;
				dP[2 * (y*w + x) + 1] = 1;
				mdP[2 * (y*w + x) + 1] = 1;

			}
		}
		x = w - 1;
		eP[y*w + x] = 3;
		dP[2 * (y*w + x) + 0] = 1;
		mdP[2 * (y*w + x) + 0] = 1;
		dP[2 * (y*w + x) + 1] = 1;
		mdP[2 * (y*w + x) + 1] = 1;
	}
	for (int x = 0; x < w; x++) {
		auto y = (h - 1);
		eP[y*w + x] = 3;
		dP[2 * (y*w + x) + 0] = 1;
		mdP[2 * (y*w + x) + 0] = 1;
		dP[2 * (y*w + x) + 1] = 1;
		mdP[2 * (y*w + x) + 1] = 1;
	}
	if (doL1) {
		//do each row, forward and backwards pass
		for (int y = 0; y < h; y++) {
			for (int x = 1; x < w; x++) {
				dP[2 * (y*w + x) + 0] = std::min<uint32_t>(dP[2 * (y*w + x) + 0], dP[2 * (y*w + x - 1) + 0] + 1);
				dP[2 * (y*w + x) + 1] = std::min<uint32_t>(dP[2 * (y*w + x) + 1], dP[2 * (y*w + x - 1) + 1] + 1);
			}
			for (int x = w - 2; x >= 0; x--) {
				dP[2 * (y*w + x) + 0] = std::min<uint32_t>(dP[2 * (y*w + x) + 0], dP[2 * (y*w + x + 1) + 0] + 1);
				dP[2 * (y*w + x) + 1] = std::min<uint32_t>(dP[2 * (y*w + x) + 1], dP[2 * (y*w + x + 1) + 1] + 1);
			}
		}
		//do each column, forward and backwards pass
		for (int x = 0; x < w; x++) {
			for (int y = 1; y < h; y++) {
				dP[2 * (y*w + x) + 0] = std::min<uint32_t>(dP[2 * (y*w + x) + 0], dP[2 * ((y - 1)*w + x) + 0] + 1);
				dP[2 * (y*w + x) + 1] = std::min<uint32_t>(dP[2 * (y*w + x) + 1], dP[2 * ((y - 1)*w + x) + 1] + 1);
			}
			for (int y = h - 2; y >= 0; y--) {
				dP[2 * (y*w + x) + 0] = std::min<uint32_t>(dP[2 * (y*w + x) + 0], dP[2 * ((y + 1)*w + x) + 0] + 1);
				dP[2 * (y*w + x) + 1] = std::min<uint32_t>(dP[2 * (y*w + x) + 1], dP[2 * ((y + 1)*w + x) + 1] + 1);
			}
		}
	} else {
		img::Img<float> b(std::max<int>(w,h)+1, 1);
		img::Img<float> p(std::max<int>(w, h) + 1, 1);
		auto boundries = b.ptr();
		auto parabolas = p.ptr();

		for (int y = 0; y < h; y++) {
			for (int c = 0; c < 2; c++) {
				int k = 0;
				parabolas[0] = 0;
				boundries[0] = (float)-INT_MAX;
				boundries[1] = (float)INT_MAX;
				for (int x = 1; x < w; x++) {
					auto f = [&](int z) { return (float)dP[2 * (y*w + z) + c]; };

					for (;; k--) {
						float dn = (2 * x - 2 * parabolas[k]);
						float s = ((f(x) + x*x) - (f((int)parabolas[k]) + square(parabolas[k]))) / (dn == 0.0f ? 1 : dn);
						if (s > boundries[k]) {
							k = k + 1;
							parabolas[k] = (float)x;
							boundries[k] = s;
							boundries[k + 1] = (float)INT_MAX;
							break;
						}
					}
				}
				k = 0;
				for (int x = 0; x < w; x++) {
					auto f = [&](int z) { return (float)dP[2 * (y*w + z) + c]; };

					while (boundries[k + 1] < x)
						k = k + 1;
					dP[2 * (y*w + x) + c] = (uint32_t)(square(x - parabolas[k]) + f((int)parabolas[k]));
				}
			}
		}
		for (int x = 0; x < w; x++) {
			for (int c = 0; c < 2; c++) {
				int k = 0;
				parabolas[0] = 0;
				boundries[0] = (float)-INT_MAX;
				boundries[1] = (float)INT_MAX;
				for (int y = 1; y <h; y++) {
					auto f = [&](int z) { return (float)dP[2 * (z*w + x) + c]; };

					for (;; k--) {
						float dn = (2 * y - 2 * parabolas[k]);
						float s = ((f(y) + y*y) - (f((int)parabolas[k]) + square(parabolas[k]))) / (dn == 0.0f ? 1 : dn);
						if (s > boundries[k]) {
							k = k + 1;
							parabolas[k] = (float)y;
							boundries[k] = s;
							boundries[k + 1] = (float)INT_MAX;
							break;
						}
					}
				}
				k = 0;
				for (int y = 0; y <h; y++) {
					auto f = [&](int z) { return (float)dP[2 * (z*w + x) + c]; };

					while (boundries[k + 1] < y)
						k = k + 1;
					dP[2 * (y*w + x) + c] = (uint32_t)(square(y - parabolas[k]) + f((int)parabolas[k]));
				}
			}
		}
	}

	
	//could also use connected component with set/union to get correct segment maximums in O(n), instead of iterative approx
	for (int iter = 0; iter < ITERNUM; iter++){
		for (int y = 0; y < h; y++) {
			for (int x = 1; x < w; x++) {
				mdP[2 * (y*w + x) + 0] = dP[2 * (y*w + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 0], std::max<uint32_t>(mdP[2 * (y*w + x) + 0], mdP[2 * (y*w + x - 1) + 0]));
				mdP[2 * (y*w + x) + 1] = dP[2 * (y*w + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 1], std::max<uint32_t>(mdP[2 * (y*w + x) + 1], mdP[2 * (y*w + x - 1) + 1]));
			}
			for (int x = w - 2; x >= 0; x--) {
				mdP[2 * (y*w + x) + 0] = dP[2 * (y*w + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 0], std::max<uint32_t>(mdP[2 * (y*w + x) + 0], mdP[2 * (y*w + x + 1) + 0]));
				mdP[2 * (y*w + x) + 1] = dP[2 * (y*w + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 1], std::max<uint32_t>(mdP[2 * (y*w + x) + 1], mdP[2 * (y*w + x + 1) + 1]));
			}
		}
		//do each column, forward and backwards pass
		for (int x = 0; x < w; x++) {
			for (int y = 1; y < h; y++) {
				mdP[2 * (y*w + x) + 0] = dP[2 * (y*w + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 0], std::max<uint32_t>(mdP[2 * (y*w + x) + 0], mdP[2 * ((y - 1)*w + x) + 0]));
				mdP[2 * (y*w + x) + 1] = dP[2 * (y*w + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 1], std::max<uint32_t>(mdP[2 * (y*w + x) + 1], mdP[2 * ((y - 1)*w + x) + 1]));
			}
			for (int y = h - 2; y >= 0; y--) {
				mdP[2 * (y*w + x) + 0] = dP[2 * (y*w + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 0], std::max<uint32_t>(mdP[2 * (y*w + x) + 0], mdP[2 * ((y + 1)*w + x) + 0]));
				mdP[2 * (y*w + x) + 1] = dP[2 * (y*w + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*w + x) + 1], std::max<uint32_t>(mdP[2 * (y*w + x) + 1], mdP[2 * ((y + 1)*w + x) + 1]));
			}
		}
	}
	const float halfScale = scaleFactor*0.5f;
	for (int i = 0; i < w*h; i++) {
		auto sw = std::min<uint32_t>(dP[2 * i + 0], dP[2 * i + 1]);
		auto lw = std::max<uint32_t>(mdP[2 * i + 0], mdP[2 * i + 1]);
		int sgn = (sw == dP[2 * i + 0]) ? 1 : -1;
		auto dst = (lw == mdP[2 * i + 0]) ? dP[2 * i + 0] : dP[2 * i + 1];
		if (p[i]) {

			float shift = (static_cast<float>(dP[2 * i + 1]) + static_cast<float>(dP[2 * i + 0]) - 2.0f) / static_cast<float>(2 * lw);

			shift = halfScale - halfScale*shift;
			int i_shift = (int)nearbyint(sgn*shift);
			s[i] = i_shift;
			p[i] = p[i] * scaleFactor + i_shift;
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
	generatePoints(input, fx, fy, px, py, output);
	return output;
}

template <int size>
inline void generateNormals_fromPoints(img::Image<float, 3> input, img::Image<float, 3> output) {
	auto height = input.height;
	auto width = input.width;
	auto points = input.ptr();
	auto normals = output.ptr();
	memset(normals, 0, width*height*sizeof(float) * 3);
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