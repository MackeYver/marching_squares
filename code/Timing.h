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
#include <stdio.h>

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif

#include "Types.h"

struct time_measure
{
    LARGE_INTEGER TotalTime;
    LARGE_INTEGER StartTime;
    u32 Count;
};

static void StartMeasure(time_measure *Measure)
{
    QueryPerformanceCounter(&Measure->StartTime);
}

static void StopMeasure(time_measure *Measure)
{
    LARGE_INTEGER EndTime;
    QueryPerformanceCounter(&EndTime);
    
    Measure->TotalTime.QuadPart += EndTime.QuadPart - Measure->StartTime.QuadPart;
    ++Measure->Count;
}

static void ConvertToMicroSeconds(time_measure *Measure, LARGE_INTEGER *Frequency)
{
    Measure->TotalTime.QuadPart *= 1000000;
    Measure->TotalTime.QuadPart /= Frequency->QuadPart;
}


union time_measurements
{
    struct
    {
        //time_measure TTotal;
        
        //time_measure TMarching;
        time_measure TBinarySum;
        time_measure TLerp;
        time_measure TAdd;
        
        //time_measure TSimplify;
        time_measure TGetLineChain;
        time_measure TMergeLines;
    };
    
    //time_measure Array[8]; 
    time_measure Array[5]; 
};

static char const *TimingNames[] =
{
    //"Total",
    
    //"Marching",
    "BinarySum",
    "Lerp",
    "Add",
    
    //"Simplify",
    "GetLineChain",
    "MergeLines"
};

static char const *TimingParentNames[] =
{
    //"Total",
    
    //"Marching",
    "Marching",
    "Marching",
    "Marching",
    
    //"Simplify",
    "Simplify",
    "Simplify",
};

static void ClearTimeMeasurements(time_measurements *M)
{
    ZeroMemory(M, sizeof(time_measurements));
}


static void CreateFile(FILE **File, char const *PathAndName)
{
    assert(File);
    //int ErrorCode = fopen_s(&FilePerf, "..\\perf\\perf.txt", "w");
    int ErrorCode = fopen_s(&(*File), PathAndName, "w");
    assert(!ErrorCode);
    ErrorCode;
}

static void WritePerfHeadersToFile(FILE **File)
{
    assert(File);
    assert(*File);
    
    fprintf(*File, "Version;Optimization;Case#;Iteration#;ParentMeasure;Measure;Time;Count\n");
}

static void WritePerfToFile(FILE *File, char const *Version, u32 Optimization, u32 CaseID, u32 IterationNo,
                            char const *ParentName, char const *Name, u64 Time, u32 Count)
{
    fprintf(File, Version);
    fprintf(File, ";");
    if (Optimization == 0)
    {
        fprintf(File, "Od;");
    }
    else if (Optimization == 1)
    {
        fprintf(File, "O1;");
    }
    else if (Optimization == 2)
    {
        fprintf(File, "O2;");
    }
    
    fprintf(File, "%u;", CaseID);
    fprintf(File, "%u;", IterationNo);
    
    fprintf(File, "%s;", ParentName);
    fprintf(File, "%s;", Name);
    fprintf(File, "%llu;", Time);
    fprintf(File, "%u", Count);
    fprintf(File, "\n");
}


static void WriteDataHeadersToFile(FILE **File)
{
    assert(File);
    assert(*File);
    
    fprintf(*File, "Version;Optimization;Case#;Iteration#;VertexCountAnte;VertexCountPost\n");
}

static void WriteDataToFile(FILE *File, char const *Version, u32 Optimization, u32 CaseID, u32 IterationNo,
                            u32 VertexCountAnte, u32 VertexCountPost)
{
    fprintf(File, Version);
    fprintf(File, ";");
    if (Optimization == 0)
    {
        fprintf(File, "Od;");
    }
    else if (Optimization == 1)
    {
        fprintf(File, "O1;");
    }
    else if (Optimization == 2)
    {
        fprintf(File, "O2;");
    }
    
    fprintf(File, "%u;", CaseID);
    fprintf(File, "%u;", IterationNo);
    
    fprintf(File, "%u;", VertexCountAnte);
    fprintf(File, "%u", VertexCountPost);
    fprintf(File, "\n");
}



#endif