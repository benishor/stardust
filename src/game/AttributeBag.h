#pragma once

#include <map>
#include <iostream>

namespace Acidrain {

    using namespace std;

    class AttributeBag {
    public:

        int getInt(const char* const key);

        void setInt(const char* const key, int value);

        float getFloat(const char* const key);

        void setFloat(const char* const key, float value);

        void clear();

    private:
        map<const char*, int> integerMap;
        map<const char*, float> floatMap;
    };
}
