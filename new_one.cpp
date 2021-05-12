#include <vector>
#include <cmath>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <stdexcept>

void swap (int& x, int& y) {
    if (x != y)
    {
        int temp = x;
        x = y;
        y = temp;
    }
}

namespace serial
{
    void act (std::vector<int>& vec,int start, int size, bool dir)
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

    void bitonic_sort(std::vector<int>& vec, int start, int size, bool dir)
    {
        if (size > 1)
        {
            int m = size/2;
            bitonic_sort(vec, start, m, 1); //ascending
            bitonic_sort(vec, start + m, m, 0); //descending
            act(vec, start, size, dir);
        }
    }
}

namespace parallel
{
    template <class T, bool dir = true, uint8_t thr_num = 8>
    void bitonic_sort(std::vector<T>& vec)
    {
        for (size_t size = 1, i = 0; (size = pow(2, i)) && size < vec.size(); i++)
        {
        }
    }
}

int main ()
{
}
