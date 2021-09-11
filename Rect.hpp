#pragma once

class Rect {
public:
	Rect(int y, int x, int width, int height) {
		this->y = y;
		this->x = x;
		this->width = width;
		this->height = height;
	}

private:
	int y;
	int x;
	int width;
	int height;

public:
	inline int getY() { return y; }
	inline void setY(int y) { this->y = y; }

	inline int getX() { return x; }
	inline void setX(int x) { this->x = x; }

	inline int getWidth() { return width; }
	inline void setWidth(int width) { this->width = width; }

	inline int getHeight() { return height; }
	inline void setHeight(int height) { this->height = height; }

};