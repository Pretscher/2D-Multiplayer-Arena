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
	inline int getY() const { return y; }
	inline void setY(int y) { this->y = y; }

	inline int getX() const { return x; }
	inline void setX(int x) { this->x = x; }

	inline int getWidth() const { return width; }
	inline void setWidth(int width) { this->width = width; }

	inline int getHeight() const { return height; }
	inline void setHeight(int height) { this->height = height; }

};