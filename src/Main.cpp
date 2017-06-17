#include "rs_camera.h"
#include "imshow.h"
#include "image.h"
#include "depth_proc.h"

int main(int argc, char* argv[])
{
	RSCamera cam;
	if (cam.Start()) {
        img::Image<float, 3> points(cam.getXDim(), cam.getYDim());
		img::Image<uint8_t, 3>  n_viz(cam.getXDim(), cam.getYDim());
		img::Img<uint8_t>  z_viz(cam.getXDim(), cam.getYDim());
		while (cam.syncNext()) {
			img::Img<uint16_t> depth(cam.getXDim(), cam.getYDim(),cam.getDepth());
			const int convexSegmentIterations = 1;
            const int zScaleFactor = 35;
            const int zToDispConst = (int)std::round(cam.disp_factor);
			auto qDepth = depth.copy();
			{
                auto p = qDepth.ptr;
				for (int i = 0; i < qDepth.width*qDepth.height; i++) {
					p[i] = p[i] ? zToDispConst / ((int)p[i]) : 0;
				}
			}
			auto oDepth = qDepth.copy();
			auto q2Depth = qDepth.copy();
			generateDequant<true>(qDepth);
			generateDequant<false>(q2Depth);

			{
				auto nc = zToDispConst * 35;
                auto p = qDepth.ptr;
                for (int i = 0; i < qDepth.width*qDepth.height; i++) {
					p[i] = p[i] ? nc / ((int)p[i]) : 0;
				}
			}

			{
				auto p = oDepth.ptr;
				for (int i = 0; i < oDepth.width*oDepth.height; i++) {
					p[i] = p[i] ? zToDispConst / ((int)p[i]) : 0;
				}
			}
            auto normals = generateNormals_FromDepth<1>(depth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy());
            auto qNrms = generateNormals_FromDepth<1>(oDepth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy());
            auto dqNrms1 = generateNormals_FromDepth<1>(qDepth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy());

			img::convertToGrey(qDepth, z_viz, (uint16_t)500, (uint16_t)2500);

			img::imshow("depth", z_viz);
            img::imshow("o_normals", img::convertToGrey(normals, -1.0f, 1.0f));
            img::imshow("l1_normals", img::convertToGrey(dqNrms1, -1.0f, 1.0f));
			img::imshow("q_normals", img::convertToGrey(qNrms, -1.0f, 1.0f));

			auto c = img::getKey();
		}
	}
}