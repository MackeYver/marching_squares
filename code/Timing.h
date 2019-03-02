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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <intrin.h>

#include "Types.h"



struct time_measure
{
    unsigned __int64 TotalCycleCount;
    unsigned __int64 StartCycleCount;
    u32 Count;
};



#if 0 
#define StartMeasure(x)
#define StopMeasure(x)
#define ConvertToMicroseconds(x)
#else
static void StartMeasure(time_measure *Measure)
{
    Measure->StartCycleCount = __rdtsc();
}

static void StopMeasure(time_measure *Measure)
{
#if 0
    LARGE_INTEGER EndTime;
    QueryPerformanceCounter(&EndTime);
    
    Measure->TotalTime.QuadPart += EndTime.QuadPart - Measure->StartTime.QuadPart;
    ++Measure->Count;
#endif
    
    unsigned __int64 CurrentCycleCount = __rdtsc();
    Measure->TotalCycleCount += CurrentCycleCount - Measure->StartCycleCount;
    ++Measure->Count;
}

#if 0
static void ConvertToMicroSeconds(time_measure *Measure, LARGE_INTEGER *Frequency)
{
    Measure->TotalTime.QuadPart *= 1000000;
    Measure->TotalTime.QuadPart /= Frequency->QuadPart;
}
#else
#define ConvertToMicroseconds(x)
#endif
#endif


union time_measurements
{
    struct
    {
        time_measure BinarySum;
        time_measure Lerp;
        time_measure Add;
        
        time_measure Forward;
        time_measure Backward;
        time_measure CalculateKey;
        time_measure Find;
        time_measure Loop;
        time_measure Concatenate;
        time_measure MergeLines;
    };
    
    time_measure Array[7]; 
};

static char const *TimingNames[] =
{
    "BinarySum",
    "Lerp",
    "Add",
    
    "Forward",
    "Backward",
    "CalculateKey",
    "Find",
    "Loop",
    "Concatenate",
    "MergeLines"
};

static char const *TimingParentNames[] =
{
    "Marching",
    "Marching",
    "Marching",
    
    "GetLineChain",
    "GetLineChain",
    "GetNextLineSegment",
    "GetNextLineSegment",
    "GetNextLineSegment",
    "GetLineChain",
    "Simplify",
};

static void ClearTimeMeasurements(time_measurements *M)
{
    memset(M, 0, sizeof(time_measurements));
}


static void CreateFile(FILE **File, char const *PathAndName)
{
    assert(File);
    //int ErrorCode = fopen_s(&FilePerf, "..\\perf\\perf.txt", "w");
    int ErrorCode = fopen_s(&(*File), PathAndName, "w");
    assert(!ErrorCode);
    ErrorCode;
}

static void WritePerfHeadersToFile(FILE *File)
{
    assert(File);
    fprintf(File, "Version;Optimization;Case#;Iteration#;ParentMeasure;Measure;Time;Count\n");
}

static void WritePerfToFile(FILE *File, char const *Version, u32 Optimization, u32 CaseID, u32 IterationNo,
                            char const *ParentName, char const *Name, u64 Time, u32 Count)
{
    assert(File);
    assert(Version);
    assert(Name);
    
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


static void WriteDataHeadersToFile(FILE *File)
{
    assert(File);
    fprintf(File, "Version;Optimization;Case#;Iteration#;VertexCountAnte;VertexCountPost\n");
}

static void WriteDataToFile(FILE *File, char const *Version, u32 Optimization, u32 CaseID, u32 IterationNo,
                            u32 VertexCountAnte, u32 VertexCountPost)
{
    assert(File);
    assert(Version);
    
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