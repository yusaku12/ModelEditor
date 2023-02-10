#pragma once

template <class T>
class Singleton
{
public:
    static T* Instance()
    {
        static T t;
        return &t;
    }
};