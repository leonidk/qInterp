#pragma once
#include <stdint.h>
#include <math.h>
#include <stddef.h>
class Camera {
public:
	virtual bool Start() = 0;
	virtual bool syncNext() = 0;
	virtual uint16_t *getDepth() = 0;
	virtual uint16_t *getRGB() = 0;
	size_t getXDim() {
		return width;
	};
	size_t getYDim() {
		return height;
	};
	float getFx() {
		return fx;
	};
	float getFy() {
		return fy;
	};
	float getPx() {
		return px;
	};
	float getPy() {
		return py;
	};

protected:
	size_t width, height;
	float px, py, fx, fy;
};
