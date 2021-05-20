#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <stdexcept>

template <class T>
void swap (T& x, T& y) {
    if (x != y)
    {
        T temp = x;
        x = y;
        y = temp;
    }
}

namespace serial
{
    template <class T>
    void act (std::vector<T>& vec,int start, int size, bool dir)
    {
        if (size > 1)
        {
            int m = size/2;
            for (int i = start; i < start + m; i++)
                if (dir == (vec[i] > vec[i+m]))
                    swap(vec[i], vec[i+m]);

            act(vec, start, m, dir);
            act(vec, start + m, m, dir);
        }
    }

    template <class T>
    void bitonic_sort(std::vector<T>& vec, int start, int size, bool dir)
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
    uint32_t num_of_threads = 0;

    bool isEven (int64_t number)
    {
        for (; !(number % 2); number /= 2);
        return (number == 1);
    }

    // nothing will change in code below if thr_num or dir will be a template 
    // but number of threads is not a template, because overwise
    // impossible to draw a table (need constant value)
    template <class T> // <class T, bool dir = true, int thr_num = 8> 
        void bitonic_sort(std::vector<T>& vec, int thr_num = 2, bool dir = true)
        {
            // checking if size == 2^n
            size_t size = vec.size();
            for (; !(size % 2); size /= 2);
            if (size != 1)
                throw std::invalid_argument("Vector length have to be 2^n size");

            // thr_num is always 2^n, overwise take < than thr_num and == 2^n
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
                    for (uint8_t i = 7; less > thr_num; less = pow(2,i), i--);
                    num_of_threads = less;

                } else 
                    num_of_threads = thr_num;
            }

            #ifdef DEBUG
            std::cout << "Number of threads: " << num_of_threads << std::endl;
            #endif

            // if too many threads or to small vec then serial sort
            // change during tests !
            uint32_t num_of_elements = vec.size() / num_of_threads;
            if (num_of_elements > 1)
            {
                std::thread* arr_thr = new std::thread [num_of_threads];
                bool d (false);
                for (uint32_t i = 0; i < num_of_threads; i++)
                {
                    d = !d;
                    arr_thr[i] = std::thread(serial::bitonic_sort<T>,
                            std::ref(vec), i * num_of_elements, num_of_elements, d);
                }

                for (uint32_t i = 0; i < num_of_threads; i++)
                    arr_thr[i].join();

                size_t size = num_of_elements;
                uint8_t tmp_thr = num_of_threads;
                uint8_t power = 1;
                while (size < (vec.size() / 2))
                {
                    for (uint8_t i = 0; i < tmp_thr / 2; i++)
                    {
                        d = !d;
                        arr_thr[i] = std::thread(serial::act<T>, std::ref(vec),
                                    i * pow(2, power) * num_of_elements, pow(2, power) * num_of_elements, d);
                    }

                    for (uint8_t i = 0; i < tmp_thr / 2; i++)
                        arr_thr[i].join();

                    size *= 2;
                    tmp_thr /= 2;
                    power++;
                }

                serial::act(vec, 0, vec.size(), dir);

            } else
                serial::bitonic_sort(vec, 0, vec.size(), dir);
        }
}

void print_test_table ()
{
    std::cout << "  ";
    for (int i = 1; i < 10; i++) // num of elements in vec
        std::cout << std::setw(6) << "2^" << i;
    for (int i = 10; i < 21; i++)
        std::cout << std::setw(5) << "2^" << i;

    std::cout << std::endl << std::endl;

    for (int i = 2; i < 13; i++) // cpu has 12 threads
    {
        std::cout << std::setw(2) << i;
        
        for (int j = 1; j < 21; j++)
        {
            // different direcitions of sorting
            bool dir;
            (parallel::isEven(j)) ? (dir = true) : (dir = false);

            std::srand(100);
            size_t size = pow(2, j);
            std::vector<int> vec_1 (size);
            for (int i = 0; i < size; i++)
                vec_1[i] = std::rand() % 1000;

            auto start_serial = std::chrono::steady_clock::now();
            serial::bitonic_sort(vec_1, 0, vec_1.size(), dir);
            auto end_serial = std::chrono::steady_clock::now();
            auto serial = std::chrono::duration_cast<std::chrono::milliseconds>(end_serial - start_serial).count();

            std::srand(100);
            std::vector<int> vec_2 (size);
            for (int i = 0; i < size; i++)
                vec_2[i] = std::rand() % 1000;

            auto start = std::chrono::steady_clock::now();
            parallel::bitonic_sort<int> (vec_2, i, dir);
            auto end = std::chrono::steady_clock::now();
            auto parallel = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            std::cout << std::setw(5) << serial - parallel << "ms";
        }

        std::cout << std::endl << std::endl;
    }
}

// custom data type
class alph
{
public:
    char a = 0;

    alph() {};
    alph(char b) : a(b) {}
    bool operator > (alph b) { return a > b.a; }
    bool operator < (alph b) { return a < b.a; }
    bool operator != (alph b) { return !(a == b.a); }
};

int main ()
{
    // Вывод: 
    //  при большом количестве элементов в векторе многопоточность работает быстрее
    //  2 потока являются оптимальными при размере вектора до 2^20
    // print_test_table();

    std::srand(std::time(0));
    std::vector<alph> test_vec (1024);
    for (auto& x : test_vec)
        x.a = std::rand() % 26 + 97;

    parallel::bitonic_sort (test_vec);

    for (auto x : test_vec)
        std::cout << x.a << ' ';

    std::cout << std::endl << std::endl;
    std::vector<int> test_vec_2 (1024);
    for (int i = 0; i < 1024; i++)
        test_vec_2[i] = std::rand() % 1000;

    parallel::bitonic_sort (test_vec_2, 2, false);
    for (auto x : test_vec_2)
        std::cout << x << ' ';
}
