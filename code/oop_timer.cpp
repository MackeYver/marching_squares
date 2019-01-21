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

#include "oop_timer.h"
#include <assert.h>
#include <stdio.h>



//
// time_measure
void time_measure::Start()
{
    if (IsRunning)
    {
        Stop();
    }
    
    QueryPerformanceCounter(&StartTime);
    IsRunning = true;
}

void time_measure::Stop()
{
    LARGE_INTEGER EndTime;
    QueryPerformanceCounter(&EndTime);
    
    TotalTime.QuadPart += EndTime.QuadPart - StartTime.QuadPart;
    IsRunning = false;
}

time_measure *time_measure::AddChild(std::string Name)
{
    assert(Children.count(Name) == 0);
    //if (Children.count(Name) == 0)
    //{
    time_measure Child;
    Child.TotalTime.QuadPart = 0;
    Child.IsRunning = false;
    Children[Name] = Child;
    //}
    
    return &Children[Name];
}

void time_measure::PrintNodeAndChilds(std::string Name, time_measure& Measure, 
                                      LARGE_INTEGER Frequency, int Padding, LARGE_INTEGER ParentTime)
{
    LARGE_INTEGER ElapsedMicroseconds;
    ElapsedMicroseconds.QuadPart = Measure.TotalTime.QuadPart * 1000000;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
    
    float Percentage = 100.0f * (float)ElapsedMicroseconds.QuadPart / (float)ParentTime.QuadPart;
    
    for (int Index = 0; Index < Padding; ++Index)
    {
        printf(" ");
    }
    printf("%-15s  %5llu (%4.1f%%)\n", Name.c_str(), ElapsedMicroseconds.QuadPart, Percentage);
    
    for (auto& it : Measure.Children)
    {
        PrintNodeAndChilds(it.first, it.second, Frequency, Padding + 2, ElapsedMicroseconds); 
    }
}



//
// oop_timer
oop_timer::oop_timer()
{
    QueryPerformanceFrequency(&Frequency);
}

time_measure *oop_timer::AddNewRoot()
{
    time_measure NewMeasure;
    NewMeasure.TotalTime.QuadPart = 0;
    NewMeasure.IsRunning = false;
    Roots.push_back(NewMeasure);
    
    return &Roots[Roots.size() - 1];
}

void oop_timer::PrintTimes()
{
    for (auto& it : Roots)
    {
        LARGE_INTEGER TotalTime;
        TotalTime.QuadPart = it.TotalTime.QuadPart * 1000000;
        TotalTime.QuadPart /= Frequency.QuadPart;
        
        it.PrintNodeAndChilds("Total", it, Frequency, 0, TotalTime);
        printf("-----------------------------\n");
    }
}