#pragma once
#include "Singleton.h"
#include <Windows.h>
#include <sstream>
/// <summary>
/// Performance Counter class. You can retrieve average frame time and the frames per second of the applcation
/// Singleton class. Access with PerformanceCounter::Instance()
/// </summary>
class PerformanceCounter : public Singleton<PerformanceCounter>
{
    LARGE_INTEGER start, end, freq;
    int fps;
    double time_passed;
    double frame_time;
    int frames_passed;
public:
    /// <summary>
    /// Begins query. Call before main program execution
    /// </summary>
    void BeginQuery()
    {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
    }
    /// <summary>
    /// Ends the query. Call after main program execution
    /// </summary>
    void EndQuery()
    {
        QueryPerformanceCounter(&end);
    }
    /// <summary>
    /// Returns the string for the queried results
    /// </summary>
    /// <returns></returns>
    std::ostringstream Results()
    {
        frame_time = (double)(end.QuadPart - start.QuadPart) / (double)(freq.QuadPart);
        std::ostringstream osts;
        osts << "ModelEditer :" << "Delta : " << frame_time << "   FPS : " << fps;
        time_passed += frame_time;
        ++frames_passed;
        if (time_passed > 1.0)
        {
            fps = frames_passed;
            time_passed = frames_passed = 0;
        }
        return osts;
    }
    /// <summary>
    /// Results the frametime between frames
    /// </summary>
    /// <returns></returns>
    float FrameTime()
    {
        return static_cast<float>(frame_time);
    }
    /// <summary>
    /// Returns the frame per second of the application
    /// </summary>
    /// <returns></returns>
    int FPS()
    {
        return fps;
    }
};