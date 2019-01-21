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

#ifndef oop_timer__h
#define oop_timer__h

#include <windows.h>

#include <map>
#include <vector>



class oop_timer;
class time_measure
{
    friend oop_timer;
    
    public:
    void Start();
    void Stop();
    
    time_measure *AddChild(std::string Name);
    
    void PrintNodeAndChilds(std::string Name, time_measure& Measure, 
                            LARGE_INTEGER Frequency, int Padding, LARGE_INTEGER ParentTime);
    
    
    protected:
    std::map<std::string, time_measure> Children;
    
    LARGE_INTEGER StartTime;
    LARGE_INTEGER TotalTime;
    
    bool IsRunning = false;
};

class oop_timer
{
    public:
    oop_timer();
    time_measure *AddNewRoot();
    void PrintTimes();
    
    
    private:
    std::vector<time_measure> Roots;
    LARGE_INTEGER Frequency;
};


#endif