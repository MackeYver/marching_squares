// 
// MIT License
// 
// Copyright (c) 2018 Marcus Larsson
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef Timing_h
#define Timing_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Types.h"

struct time_measure
{
    LARGE_INTEGER TotalTime;
    LARGE_INTEGER StartTime;
    u32 Count;
};

inline void StartMeasure(time_measure *Measure)
{
    QueryPerformanceCounter(&Measure->StartTime);
}

inline void StopMeasure(time_measure *Measure)
{
    LARGE_INTEGER EndTime;
    QueryPerformanceCounter(&EndTime);
    
    Measure->TotalTime.QuadPart += EndTime.QuadPart - Measure->StartTime.QuadPart;
    ++Measure->Count;
}

inline void ConvertToMicroSeconds(time_measure *Measure, LARGE_INTEGER *Frequency)
{
    Measure->TotalTime.QuadPart *= 1000000;
    Measure->TotalTime.QuadPart /= Frequency->QuadPart;
}

#endif