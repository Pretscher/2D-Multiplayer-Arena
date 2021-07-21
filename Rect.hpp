#pragma once

class Rect {
public:
	Rect(int row, int col, int width, int height) {
		this->row = row;
		this->col = col;
		this->width = width;
		this->height = height;
	}

private:
	int row;
	int col;
	int width;
	int height;

public:
	inline int getRow() { return row; }
	inline void setRow(int row) { this->row = row; }

	inline int getCol() { return col; }
	inline void setCol(int col) { this->col = col; }

	inline int getWidth() { return width; }
	inline void setWidth(int width) { this->width = width; }

	inline int getHeight() { return height; }
	inline void setHeight(int height) { this->height = height; }

};