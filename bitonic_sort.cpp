// -pthread flag for gcc compiler !

#include <vector>
#include <cmath>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <stdexcept>

// Array of threads for a half of a vector
class Arr_of_thr
{
    std::thread* _arr;
    uint8_t _cap;
    uint8_t _size;
    // if thr is joined
    // std::vector<bool> _bitset;

public:

    Arr_of_thr() {};

    void init (uint8_t cap)
    { //_bitset.resize(_cap); }
        _arr = new std::thread [cap];
        _cap = cap;
        _size = 0;
    }

    template <class Function>
    void add (Function& func, 
            std::vector<int>& vec, int start, int size, bool dir)
    { 
        if (_size != _cap)
        {
            // int16_t i = -1;
            // for (; i < _cap && _bitset[i] == 1; i++);
            // if thread isn't detached => terminate
            _size ++;
            _arr[_size] = std::thread(func, std::ref(vec), start, size, dir);
            _arr[_size].detach();
        }
    }
    
    uint8_t size() { return _size; }
    uint8_t capacity() { return _cap; }

    ~Arr_of_thr() { 
        /*
        for (int i = 0; i < _size; i++)
            _arr[i].join();
            */

        delete [] _arr; 
    }
};

uint8_t num_of_threads = 0;
Arr_of_thr thr_left_half;
Arr_of_thr thr_right_half;

// serial bitonic sort
// from 4th lab
////////////////////////////////////////////////

void swap (int& x, int& y) {
	int temp = x;
	x = y;
	y = temp;
}

void act (std::vector<int>& vec, int start, int size, bool dir)
{
	if (size > 1)
	{
	    int m = size/2;
		for (int i = start; i < start + m; i++)
			if (dir == (vec[i]>vec[i+m]))
					swap(vec[i], vec[i+m]);

		act(vec, start, m, dir);
		act(vec, start + m, m, dir);
	}
}

//////////////////////////////////////////////////

void bitonic_sort_backend (std::vector<int>& vec, int start, int size, bool dir)
{
	if (size > 1)
	{
		int m = size/2;

        (thr_left_half.size() != thr_left_half.capacity()) ?
        thr_left_half.add(bitonic_sort_backend, vec, start, m, 1) :
		bitonic_sort_backend (vec, start, m, 1); //ascending

        (thr_right_half.size() != thr_right_half.capacity()) ?
        thr_right_half.add(bitonic_sort_backend, vec, start + m, m, 0) :
		bitonic_sort_backend (vec, start + m, m, 0); //descending

		act(vec, start, size, dir);
	}
}

template <class T, bool dir = true, uint8_t thr_num = 8>
void bitonic_sort(std::vector<T>& vec)
{
    // checking if size == 2^n
    size_t size = vec.size();
    for (; !(size % 2); size /= 2);
    if (size != 1)
        throw std::invalid_argument("Vector length have to be 2^n size");

    /*
    if (thr_num == 0)
        num_of_threads = 1;
    else {
        // checking if thr_num == 2^n
        uint8_t tmp = thr_num;
        for (; !(tmp% 2); tmp /= 2);
        // if thr_num != 2^n then take first (number = 2^k) < thr_num
        if (tmp != 1) 
        {
            uint16_t less = pow(2, 8);
            for (uint8_t i = 7; less > tmp; less = pow(2,i), i--);
            num_of_threads = less;

        } else 
            num_of_threads = thr_num;
    }
    */
    
    (thr_num == 0) ? num_of_threads = 1 : num_of_threads = thr_num;

    if (num_of_threads != 1)
    {
        (!(num_of_threads - 1) % 2) ? thr_left_half.init((num_of_threads - 1)/ 2) :
            thr_left_half.init ((num_of_threads - 1) / 2 + 1);

        thr_right_half.init((num_of_threads - 1) / 2);
    }

    bitonic_sort_backend (vec, 0, vec.size(), dir);
}

int main ()
{
    std::srand(100);
    std::vector<int> vec_2 (512);
    for (int i = 0; i < 512; i++)
        vec_2[i] = std::rand() % 100;

    bitonic_sort<int, true, 12> (vec_2);
    for (auto x : vec_2)
        std::cout << x << ' ';
}
