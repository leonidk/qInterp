#include "oni_camera.h"
#include "imshow.h"
#include "image.h"
#include "depth_proc.h"

int main(int argc, char* argv[])
{
	ONICamera cam;
	if (cam.Start()) {
		img::Image<float, 3> points(cam.getXDim(), cam.getYDim()), normals(cam.getXDim(), cam.getYDim());
		img::Image<uint8_t, 3>  n_viz(cam.getXDim(), cam.getYDim());
		img::Img<uint8_t>  z_viz(cam.getXDim(), cam.getYDim());
		while (cam.syncNext()) {
			img::Img<uint16_t> depth(cam.getXDim(), cam.getYDim(),cam.getDepth());

			const int ITERNUM = 4;
			const int zToDispConst = 352400; //352400
			//values are from 0 to ~8000
			auto qDepth = depth.copy();
			{
				auto p = qDepth.ptr();
				for (int i = 0; i < qDepth.width*qDepth.height; i++) {
					p[i] = p[i] ? zToDispConst / ((int)p[i]) : 0;
				}
			} 
			{
				auto p = qDepth.ptr();

				img::Img<uint8_t> edges(depth.width, depth.height);
				img::Img<uint32_t> shift(depth.width, depth.height);
				auto s = shift.ptr();
				memset(s, 0, depth.width*depth.height*4);

				memset(edges.data.get(), 0, depth.width*depth.height);
				auto eP = edges.ptr();



				img::Image<uint32_t, 2> distance(depth.width, depth.height);
				img::Image<uint32_t, 2> minDistance(depth.width,depth.height);
				auto dP = distance.ptr();
				auto mdP = minDistance.ptr();
				for (int i = 0; i < 2 * distance.width*distance.height; i++) {
					dP[i] = INT_MAX;
				}
				for (int i = 0; i < 2 * minDistance.width*minDistance.height; i++) {
					mdP[i] = 0;
				}
				for (int x = 0; x < depth.width; x++) {
					auto y = 0;
					eP[y*depth.width + x] = 3;
					dP[2 * (y*depth.width + x) + 0] = 1;
					mdP[2 * (y*depth.width + x) + 0] = 1;
					dP[2 * (y*depth.width + x) + 1] = 1;
					mdP[2 * (y*depth.width + x) + 1] = 1;
				}
				for (int y = 1; y < depth.height - 1; y++) {
					int x = 0;
					eP[y*depth.width + x] = 3;
					dP[2 * (y*depth.width + x) + 0] = 1;
					mdP[2 * (y*depth.width + x) + 0] = 1;
					dP[2 * (y*depth.width + x) + 1] = 1;
					mdP[2 * (y*depth.width + x) + 1] = 1;
					for (int x = 1; x < depth.width - 1; x++) {
						int d = p[y*depth.width + x];
						int dn = p[(y - 1)*depth.width + x];
						int ds = p[(y + 1)*depth.width + x];
						int dw = p[y*depth.width + (x - 1)];
						int de = p[y*depth.width + (x + 1)];

						if (!d) {
							eP[y*depth.width + x] |= 1;
							dP[2 * (y*depth.width + x) + 0] = 1;
							eP[y*depth.width + x] |= 2;
							dP[2 * (y*depth.width + x) + 1] = 1;
						}
						if ((dw - d) >= 1 || (de - d) >= 1 || (dn - d) >= 1 || (ds - d) >= 1) {
							eP[y*depth.width + x] |= 1;
							dP[2 * (y*depth.width + x) + 0] = 1;
							mdP[2 * (y*depth.width + x) + 0] = 1;
						}
						if ((dw - d) <= -1 || (de - d) <= -1 || (dn - d) <= -1 || (ds - d) <= -1) {
							eP[y*depth.width + x] |= 2;
							dP[2 * (y*depth.width + x) + 1] = 1;
							mdP[2 * (y*depth.width + x) + 1] = 1;

						}
						//if (abs(dn - d) > 1 || abs(ds - d) > 1 ||abs(dw - d) > 1 || abs(de - d) > 1)
						//	eP[y*depth.width + x] |= 4;

					}
					x = depth.width-1;
					eP[y*depth.width + x] = 3;
					dP[2 * (y*depth.width + x) + 0] = 1;
					mdP[2 * (y*depth.width + x) + 0] = 1;
					dP[2 * (y*depth.width + x) + 1] = 1;
					mdP[2 * (y*depth.width + x) + 1] = 1;
				}
				for (int x = 0; x < depth.width; x++) {
					auto y = (depth.height - 1);
					eP[y*depth.width + x] = 3;
					dP[2 * (y*depth.width + x) + 0] = 1;
					mdP[2 * (y*depth.width + x) + 0] = 1;
					dP[2 * (y*depth.width + x) + 1] = 1;
					mdP[2 * (y*depth.width + x) + 1] = 1;
				}
				//do each row, forward and backwards pass
				for (int y = 0; y < depth.height; y++) {
					for (int x = 1; x < depth.width; x++) {
						dP[2 * (y*depth.width + x) + 0] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 0], dP[2 * (y*depth.width + x - 1) + 0] + 1);
						dP[2 * (y*depth.width + x) + 1] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 1], dP[2 * (y*depth.width + x - 1) + 1] + 1);
					}
					for (int x = depth.width-2; x >= 0; x--) {
						dP[2 * (y*depth.width + x) + 0] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 0], dP[2 * (y*depth.width + x + 1) + 0] + 1);
						dP[2 * (y*depth.width + x) + 1] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 1], dP[2 * (y*depth.width + x + 1) + 1] + 1);
					}
				}
				//do each column, forward and backwards pass
				for (int x = 0; x < depth.width; x++) {
					for (int y = 1; y < depth.height; y++) {
						dP[2 * (y*depth.width + x) + 0] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 0], dP[2 * ((y-1)*depth.width + x) + 0] + 1);
						dP[2 * (y*depth.width + x) + 1] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 1], dP[2 * ((y-1)*depth.width + x) + 1] + 1);
					}
					for (int y = depth.height - 2; y >= 0; y--) {
						dP[2 * (y*depth.width + x) + 0] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 0], dP[2 * ((y+1)*depth.width + x) + 0] + 1);
						dP[2 * (y*depth.width + x) + 1] = std::min<uint32_t>(dP[2 * (y*depth.width + x) + 1], dP[2 * ((y+1)*depth.width + x) + 1] + 1);
					}
				}

				for (int iter = 0; iter < ITERNUM; iter++){
					for (int y = 0; y < depth.height; y++) {
						for (int x = 1; x < depth.width; x++) {
							mdP[2 * (y*depth.width + x) + 0] = dP[2 * (y*depth.width + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 0], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 0], mdP[2 * (y*depth.width + x - 1) + 0]));
							mdP[2 * (y*depth.width + x) + 1] = dP[2 * (y*depth.width + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 1], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 1], mdP[2 * (y*depth.width + x - 1) + 1]));
						}
						for (int x = depth.width - 2; x >= 0; x--) {
							mdP[2 * (y*depth.width + x) + 0] = dP[2 * (y*depth.width + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 0], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 0], mdP[2 * (y*depth.width + x + 1) + 0]));
							mdP[2 * (y*depth.width + x) + 1] = dP[2 * (y*depth.width + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 1], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 1], mdP[2 * (y*depth.width + x + 1) + 1]));
						}
					}
					//do each column, forward and backwards pass
					for (int x = 0; x < depth.width; x++) {
						for (int y = 1; y < depth.height; y++) {
							mdP[2 * (y*depth.width + x) + 0] = dP[2 * (y*depth.width + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 0], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 0], mdP[2 * ((y - 1)*depth.width + x) + 0]));
							mdP[2 * (y*depth.width + x) + 1] = dP[2 * (y*depth.width + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 1], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 1], mdP[2 * ((y - 1)*depth.width + x) + 1]));
						}
						for (int y = depth.height - 2; y >= 0; y--) {
							mdP[2 * (y*depth.width + x) + 0] = dP[2 * (y*depth.width + x) + 0] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 0], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 0], mdP[2 * ((y + 1)*depth.width + x) + 0]));
							mdP[2 * (y*depth.width + x) + 1] = dP[2 * (y*depth.width + x) + 1] == 1 ? 1 : std::max<uint32_t>(dP[2 * (y*depth.width + x) + 1], std::max<uint32_t>(mdP[2 * (y*depth.width + x) + 1], mdP[2 * ((y + 1)*depth.width + x) + 1]));
						}
					}
				}
				auto oDepth = qDepth.copy();
				{
					auto o = oDepth.ptr();
					for (int i = 0; i < oDepth.width*oDepth.height; i++) {
						o[i] *= 10;
					}
				}
				for (int i = 0; i < depth.width*depth.height; i++) {
					auto sw = std::min<uint32_t>(dP[2 * i + 0], dP[2 * i + 1]);
					auto lw = std::max<uint32_t>(mdP[2 * i + 0], mdP[2 * i + 1]);
					int sgn = (sw == dP[2 * i + 0]) ? 1 : -1;
					auto dst = (lw == mdP[2 * i + 0]) ? dP[2 * i + 0] : dP[2 * i + 1];
					if (p[i]) {
						//if (sw == 1) {
						//	auto shift = 0; sgn*(dst - 1 + (lw - 2)) / (lw - 1);
						//	s[i] = shift;
						//	p[i] = p[i] * 10 + shift;
						//}
						//else 
						{
							//auto shiftA = (5*(dP[2 * i + 0] - 1))/ (mdP[2 * i + 0]);
							//auto shiftB = (5*(dP[2 * i + 1] - 1))/ (mdP[2 * i + 1]);
							//auto shift = -shiftA + shiftB;
							//auto shift = 5 * ((mdP[2 * i + 0] - dP[2 * i + 0] + (mdP[2 * i + 0]-1) ) / (mdP[2 * i + 0]));
							// shift = Max/Max = 0;
							//float shift = static_cast<float>(mdP[2 * i + 0] - dP[2 * i + 0]) / static_cast<float>(mdP[2 * i + 0]);
							float shift = (static_cast<float>(dP[2 * i + 1]) + static_cast<float>(dP[2 * i + 0])) / static_cast<float>(2*lw);

							shift = 5.0f-5.0f*shift;
							shift = nearbyint(sgn*shift);
							s[i] = shift;

							p[i] = p[i] * 10 +shift;
						}
					}
					else {
						s[i] = 0;
						p[i] *= 10;
					}
				}
				//algorithm steps:
				// distance transform from positive edge (+ max for segment)
				//  do over row
				//	do over cols
				// distance transform from negative edge ( + max for segment)
				//  do over row
				//  do over col
				// interpolate
				// if one sided
				//  x = val +/- dist/max
				// if two sided
				//  x 

				//for (int i = 0; i < qDepth.width*qDepth.height; i++) {
				//	p[i] *= 10;
				//}
			}
			{
				auto nc = zToDispConst * 10;
				auto p = qDepth.ptr();
				for (int i = 0; i < qDepth.width*qDepth.height; i++) {
					p[i] = p[i] ? nc / ((int)p[i]) : 0;
				}
			}
			generatePoints(qDepth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy(), points);
			generateNormals_fromPoints<1>(points, normals);
			auto oPts = generatePoints(depth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy());
			auto oNrms = generateNormals_fromPoints<1>(oPts);
			img::convertToGrey(qDepth, z_viz, (uint16_t)500, (uint16_t)2500);
			img::convertToGrey(normals, n_viz,-1.0f,1.0f);

			img::imshow("depth", z_viz);
			img::imshow("normals", n_viz);

			auto c = img::getKey();
		}
	}
}