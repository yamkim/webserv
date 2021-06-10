#ifndef ARRAY_HPP
#define ARRAY_HPP

template <typename T>
class Array {
	private:
		T * _arr;
		unsigned int _size;
	public:
		Array() {
			_arr = new T[0]();
			_size = 0;
		}

		Array(unsigned int n) {
			_arr = new T[n]();
			_size = n;
		}

		~Array() {
			delete [] _arr;
		}

		T & operator[](unsigned int i) {
			return (_arr[i]);
		}

		unsigned int size(void) const {
			return (_size);
		}
		
		T * getArray(void) {
			return (_arr);
		}

		void removeElement(unsigned int pos) {
			T * newArr = new T[_size - 1]();
			for (int i = 0; i < pos; i++)
				newArr[i] = _arr[i];
			for (int i = pos + 1; i < _size; i++)
				newArr[i - 1] = _arr[i];
			delete [] _arr;
			_size--;
			_arr = newArr;
		}

		void appendElement(T & element) {
			T * newArr = new T[_size + 1]();
			for (int i = 0; i < _size; i++)
				newArr[i] = _arr[i];
			newArr[_size] = element;
			delete [] _arr;
			_size++;
			_arr = newArr;
		}
};

#endif
