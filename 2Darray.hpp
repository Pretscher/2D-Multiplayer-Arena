#pragma once
#include <iostream>
#include <vector>

template<class T> class array2D {
public:
	array2D<T>() {
		initialized = false;
	}

	array2D<T>(unsigned int rows, unsigned int cols) {
		rowCount = rows;
		colCount = cols;
		data = new T[rowCount * colCount];

		initialized = true;
	}

	~array2D<T>() {
		if (initialized == true) {
			delete[] data;
		}
	}

	inline unsigned int getRows() const {
		return rowCount;
	}
	inline unsigned int getCols() const {
		return colCount;
	}

	inline T getData(unsigned int row, unsigned int col) const {
		return data[row * colCount + col];
	}

	inline void setData(unsigned int row, unsigned int col, T value) {
		data[row * colCount + col] = value;
	}

	inline int getFlattenedDataSize() const {
		return rowCount * colCount;
	}

	//&data[somewhere] returns a pointer to the memory address of the int at [somewhere]. This address is in our one-dimensional "data" array,
	//which means that if you call [] on the returned pointer, the pointer starts at [somewhere] in our "data" array. This allows us to call
	//another [somewhere2] to go somewhere2 steps further in the "data" array, which would then be the column amount. The array is mapped
	//[rows][cols], so in order to get the row we need to multiply the index with the column count to get to the right row. 
	inline T* operator[](unsigned int index) {
		return &(data[index * colCount]);
	}

	//you can only get from constant object, not change any values in the returned pointer
	const inline T* operator[](unsigned int index) const {// (automatically chosen func if object is constant)
		return &(data[index * colCount]);
	}

	inline T* getFlattenedData() {
		return data;
	}

	array2D<T>(const array2D<T>& toCopy) {
		if (toCopy.initialized == true) {
			initialized = true;
			data = toCopy.data;
			rowCount = toCopy.rowCount;
			colCount = toCopy.colCount;
		}
		else {
			initialized = false;
		}
	}

	array2D<T>(array2D<T>&& toMove) {
		if (toMove.initialized == true) {
			initialized = true;
			data = toMove.data;
			rowCount = toMove.rowCount;
			colCount = toMove.colCount;
		}
		else {
			initialized = false;
		}
	}

private:


	unsigned int rowCount, colCount;
	T* data;
	bool initialized;
};