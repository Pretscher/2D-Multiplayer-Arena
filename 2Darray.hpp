#pragma once
#include <iostream>
#include <vector>

template<class T> class array2D {
public:
	array2D() {};

	array2D(unsigned int rows, unsigned int cols) {
		this->rowCount = rows;
		this->colCount = cols;
		this->data = vector<int>(rowCount * colCount);
	}

	inline unsigned int getRows() const {
		return rowCount;
	}
	inline unsigned int getCols() const {
		return colCount;
	}

	inline T getData(int row, int col) const {
		return data.data()[row * colCount + col];
	}

	inline int getFlattenedDataSize() const {
		return rowCount * colCount;
	}

	inline T* operator[](std::size_t index) {
		if (index >= rowCount) {
			std::cout << "Row out of bounds for 2D-Array";
			std::exit(0);
		}
		return &(data.data()[index * colCount]);
	}

	inline T* getFlattenedData() {
		return data.data();
	}

private:
	unsigned int rowCount, colCount;
	vector<T> data;
};