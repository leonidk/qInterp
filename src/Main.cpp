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
			const int convexSegmentIteratons = 4;
			const int zScaleFactor = 10;
			const int zToDispConst = 3524000 / zScaleFactor;
			auto qDepth = depth.copy();
			{
				auto p = qDepth.ptr();
				for (int i = 0; i < qDepth.width*qDepth.height; i++) {
					p[i] = p[i] ? zToDispConst / ((int)p[i]) : 0;
				}
			}
			auto oDepth = qDepth.copy();
			generateDequant<zScaleFactor, convexSegmentIteratons>(qDepth);
			{
				auto nc = zToDispConst * zScaleFactor;
				auto p = qDepth.ptr();
				for (int i = 0; i < qDepth.width*qDepth.height; i++) {
					p[i] = p[i] ? nc / ((int)p[i]) : 0;
				}
			}
			{
				auto p = oDepth.ptr();
				for (int i = 0; i < oDepth.width*oDepth.height; i++) {
					p[i] = p[i] ? zToDispConst / ((int)p[i]) : 0;
				}
			}
			generatePoints(qDepth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy(), points);
			generateNormals_fromPoints<1>(points, normals);
			auto oPts = generatePoints(oDepth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy());
			auto oNrms = generateNormals_fromPoints<1>(oPts);
			img::convertToGrey(qDepth, z_viz, (uint16_t)500, (uint16_t)2500);
			img::convertToGrey(normals, n_viz,-1.0f,1.0f);

			img::imshow("depth", z_viz);
			img::imshow("normals", n_viz);
			img::imshow("o_normals", img::convertToGrey(oNrms, -1.0f, 1.0f));

			auto c = img::getKey();
		}
	}
}