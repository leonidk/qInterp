#pragma once
#include "image.h"

#include <cmath>

inline void generatePoints(img::Img<uint16_t> input, const float fx, const float fy, const float px, const float py, img::Image<float, 3> output) {
    auto halfX = px;
    auto halfY = py;
    auto cX = 1.0f / fx;
    auto cY = 1.0f / fy;
    auto depth = input.ptr;
    auto points = output.ptr;
    for (int i = 0; i < input.height; i++) {
        for (int j = 0; j < input.width; j++) {
            const auto z = depth[(i * input.width + j)];
            points[3 * (i * input.width + j)] = (j - halfX) * z * cX;
            points[3 * (i * input.width + j) + 1] = (i - halfY) * z * cY;
            points[3 * (i * input.width + j) + 2] = z;
        }
    }
}
template <bool doL1 = true, bool check = true>
inline img::Img<uint32_t> distTransform(img::Img<uint8_t> edges, img::Img<uint8_t> oedges)
{
    auto w = edges.width;
    auto h = edges.height;
    img::Img<uint32_t> dist(w, h);

    for (int i = 0; i < w*h; i++) {
        dist(i) = edges(i) ? 1 : std::max(w,h);
    }

    if (doL1) {
        //do each row, forward and backwards pass
        for (int y = 0; y < h; y++) {
            for (int x = 1; x < w; x++) {
                if (!check || oedges(y, x) == 0) {
                    dist(y, x) = std::min<uint32_t>(dist(y, x), 2 + dist(y, x - 1));
                }
            }
            for (int x = w - 2; x >= 0; x--) {
                if (!check || oedges(y, x) == 0) {
                    dist(y, x) = std::min<uint32_t>(dist(y, x), 2 + dist(y, x + 1));
                }
            }
        }
        //do each column, forward and backwards pass
        for (int x = 0; x < w; x++) {
            for (int y = 1; y < h; y++) {
                if (!check || oedges(y, x) == 0) {
                    dist(y, x) = std::min<uint32_t>(dist(y, x), 2 + dist(y - 1, x));
                }
            }
            for (int y = h - 2; y >= 0; y--) {
                if (!check || oedges(y,x) == 0 ) {
                    dist(y, x) = std::min<uint32_t>(dist(y, x), 2 + dist(y + 1, x));
                }
            }
        }
    }
    else {
        // I think this wasn't working last I tried it
        /* 
        img::Img<float> b(std::max<int>(w, h) + 1, 1);
        img::Img<float> p(std::max<int>(w, h) + 1, 1);
        auto boundries = b.ptr;
        auto parabolas = p.ptr;

        for (int y = 0; y < h; y++) {
                int k = 0;
                parabolas[0] = 0;
                boundries[0] = (float)-INT_MAX;
                boundries[1] = (float)INT_MAX;
                for (int x = 1; x < w; x++) {
                    auto f = [&dist, &y](int z) { return (float)dist(y, z); };

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
                    auto f = [&dist, &y](int z) { return (float)dist(y, z); };

                    while (boundries[k + 1] < x)
                        k = k + 1;
                    dist(y,x) = (uint32_t)(square(x - parabolas[k]) + f((int)parabolas[k]));
                }
            
        }
        for (int x = 0; x < w; x++) {
                int k = 0;
                parabolas[0] = 0;
                boundries[0] = (float)-INT_MAX;
                boundries[1] = (float)INT_MAX;
                for (int y = 1; y < h; y++) {
                    auto f = [&dist,&x](int z) { return (float)dist(z, x); };

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
                for (int y = 0; y < h; y++) {
                    auto f = [&dist, &x](int z) { return (float)dist(z, x); };

                    while (boundries[k + 1] < y)
                        k = k + 1;
                    dist(y, x) = (uint32_t)(square(y - parabolas[k]) + f((int)parabolas[k]));
                }
            } */
        
    }
    return dist;
}

template <bool doL1 = true>
inline img::Img<float> generateDequant(img::Img<uint16_t> input)
{
    auto p = input.ptr;
    auto w = input.width;
    auto h = input.height;

    img::Img<uint8_t> edgesN(w, h, (uint8_t)0); // negative
    img::Img<uint8_t> edgesP(w, h, (uint8_t)0); // positive
    img::Img<uint8_t> edgesD(w, h, (uint8_t)0); // discon
    img::Img<float> output(w, h, 0.0f);
    // for the first row
    for (int x = 0; x < w; x++) {
        auto y = 0;
        edgesN(y, x) = edgesP(y, x) = edgesD(y, x) = 1;
    }
    // for the rest of the image
    for (int y = 1; y < h ; y++) {
        //first col
        int x = 0;
        edgesN(y, x) = edgesP(y, x) = edgesD(y, x) = 1;

        for (int x = 1; x < w ; x++) {
            int d  = input(y, x);
            int dn = input(y - 1, x);
            int dw = input(y, x - 1);
   

            if (!d) {
                edgesD(y, x) = 1;
            }
            if (dw < d) {
                edgesN(y, x) = 1;
                edgesP(y, x - 1) = 1;
            }
            if (dw > d) {
                edgesP(y, x) = 1;
                edgesN(y, x - 1) = 1;
            }
            if (dn < d) {
                edgesN(y, x) = 1;
                edgesP(y-1, x) = 1;
            }
            if (dn > d) {
                edgesP(y, x) = 1;
                edgesN(y-1,x) = 1;
            }
        }

    }

    auto nDist = distTransform<doL1,true>(edgesN,edgesP);
    auto pDist = distTransform<doL1,true>(edgesP,edgesN);
    auto dDist = distTransform<doL1,false>(edgesD,edgesN);

    for (int i = 0; i < w*h; i++) {
        if (!input(i))
            continue;
        if (nDist(i) < pDist(i) && dDist(i) < pDist(i)) {
            output(i) = input(i) - 0.5f*(dDist(i)) / static_cast<float>(dDist(i) + nDist(i));
        } else if (nDist(i) < pDist(i) && dDist(i) < pDist(i)) {
            output(i) = input(i) + 0.5f*(dDist(i)) / static_cast<float>(dDist(i) + pDist(i));
        } else { 
            output(i) = input(i) + (0.5f*nDist(i) - 0.5f*pDist(i)) / static_cast<float>(pDist(i) + nDist(i));
        }
    }
    return output;
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


template <int size, typename T>
inline void generateNormals_FromDepth(const T *depth, const int width, const int height, const float fx, const float fy, const float ppx, const float ppy, float *normals) {
    const auto cX = 1.0f / fx;
    const auto cY = 1.0f / fy;
    const auto xStep = cX * size;
    const auto yStep = cY * size;
    auto halfX = ppx;
    auto halfY = ppy;
    const auto xyStep = xStep * yStep;
    for (int i = size; i < height - size; i++) {
        for (int j = size; j < width - size; j++) {
            if (!depth[i * width + j])
                continue;
            const auto cDepth = depth[i * width + j];
            float pc[] = { cX*(j - halfX)*cDepth, cY*(i - halfY)*cDepth, (float)cDepth };
            float outNorm[3] = { 0, 0, 0 };
            int count = 0;

            if (depth[i * width + j + size] && depth[(i + size) * width + j]) {
                const auto xDepth = depth[i * width + j + size];
                const auto yDepth = depth[(i + size) * width + j];
                const auto diffXZ = xDepth - cDepth;
                const auto diffYZ = yDepth - cDepth;
                float px[] = { cX*(j - halfX + size)*xDepth, cY*(i - halfY)*xDepth, (float)xDepth };
                float py[] = { cX*(j - halfX)*yDepth, cY*(i - halfY + size)*yDepth, (float)yDepth };

                // cX*(5*z +j*(z-z2));
                float v1[] = { px[0] - pc[0], px[1] - pc[1], px[2] - pc[2] };
                float v2[] = { py[0] - pc[0], py[1] - pc[1], py[2] - pc[2] };

                float v3[3];
                cross(v1, v2, v3);

                //possible optimization to unroll all operations
                //const float v11[] = { cX*(diffXZ*(j - halfX) + size*xDepth), cY*diffXZ*(i - halfY), (float)diffXZ };
                //const float v22[] = { cX*diffYZ*(j - halfX), cY*(diffYZ*(i - halfY) + size*yDepth), (float)diffYZ };
                //float v33[3];
                //cross(v11, v22, v33);

                normalize(v3);
                outNorm[0] += v3[0];
                outNorm[1] += v3[1];
                outNorm[2] += v3[2];
                count++;
            }
            if (depth[i * width + j - size] && depth[(i + size) * width + j]) {
                const auto xDepth = depth[i * width + j - size];
                const auto yDepth = depth[(i + size) * width + j];
                const auto diffXZ = xDepth - cDepth;
                const auto diffYZ = yDepth - cDepth;
                float px[] = { cX*(j - halfX - size)*xDepth, cY*(i - halfY)*xDepth, (float)xDepth };
                float py[] = { cX*(j - halfX)*yDepth, cY*(i - halfY + size)*yDepth, (float)yDepth };

                // cX*(5*z +j*(z-z2));
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
            if (depth[i * width + j + size] && depth[(i - size) * width + j]) {
                const auto xDepth = depth[i * width + j + size];
                const auto yDepth = depth[(i - size) * width + j];
                const auto diffXZ = xDepth - cDepth;
                const auto diffYZ = yDepth - cDepth;
                float px[] = { cX*(j - halfX + size)*xDepth, cY*(i - halfY)*xDepth, (float)xDepth };
                float py[] = { cX*(j - halfX)*yDepth, cY*(i - halfY - size)*yDepth, (float)yDepth };

                // cX*(5*z +j*(z-z2));
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
            if (depth[i * width + j - size] && depth[(i - size) * width + j]) {
                const auto xDepth = depth[i * width + j - size];
                const auto yDepth = depth[(i - size) * width + j];
                const auto diffXZ = xDepth - cDepth;
                const auto diffYZ = yDepth - cDepth;
                float px[] = { cX*(j - halfX - size)*xDepth, cY*(i - halfY)*xDepth, (float)xDepth };
                float py[] = { cX*(j - halfX)*yDepth, cY*(i - halfY - size)*yDepth, (float)yDepth };

                // cX*(5*z +j*(z-z2));
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
            }
        }
    }
}

template <int size,typename T>
inline img::Image<float, 3> generateNormals_FromDepth(img::Img<T> input, const float fx, const float fy, const float px, const float py) {
    img::Image<float, 3> output(input.width, input.height);
    generateNormals_FromDepth<size>(input.ptr, input.width, input.height, fx, fy, px, py, output.ptr);
    return output;
}

template <int size>
inline void generateNormals_fromPoints(img::Image<float, 3> input, img::Image<float, 3> output) {
    auto height = input.height;
    auto width = input.width;
    auto points = input.ptr;
    auto normals = output.ptr;
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

