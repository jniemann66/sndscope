#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include <array>

template<typename FloatType, size_t size>
class MovingAverage
{
    size_t index{0ull};
    FloatType movingTotal{0.0};
    static constexpr FloatType r{1.0 / std::max<FloatType>(1.0, size)};
    std::array<FloatType, size> history;

public:
    MovingAverage()
    {
        history.fill(0.0);
    }

    FloatType get(FloatType input)
    {
        movingTotal -= history[index];
        movingTotal += (history[index] = input);
        if(++index == size) {
            index = 0ull;
        }
        return r * movingTotal;
    }
};

#endif // MOVINGAVERAGE_H
