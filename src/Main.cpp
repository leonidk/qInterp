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

			generatePoints(depth, cam.getFx(), cam.getFy(), cam.getPx(), cam.getPy(), points);
			generateNormals_fromPoints<1>(points,normals);

			img::convertToGrey(depth, z_viz, (uint16_t)500, (uint16_t)2500);
			img::convertToGrey(normals, n_viz,-1.0f,1.0f);

			img::imshow("depth", z_viz);
			img::imshow("normals", n_viz);

			auto c = img::getKey();
		}
	}
}