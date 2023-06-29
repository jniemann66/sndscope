#ifndef DELAYLINE_H
#define DELAYLINE_H

#include <array>

template<typename T, size_t size>
class DelayLine
{
    std::array<T, size> history;
    size_t index{0ull};

public:
    DelayLine()
    {
        history.fill(0.0);
    }

    T get(const T input) {
        T output = history[index];
        history[index] = input;
        if(++index == size) {
            index = 0ull;
        }
        return output;
    }
};


#endif // DELAYLINE_H
