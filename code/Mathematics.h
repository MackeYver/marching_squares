//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
//

//
// Note:
// - Un-optimized code...
//

#ifndef Mathematics_h
#define Mathematics_h

#include <stdint.h>
#include <math.h>


//
// Typedefs
typedef uint8_t u8;
typedef uint32_t u32;

#define f32Max FLT_MAX
#define f32Min FLT_MIN
typedef float f32;

typedef u32 b32;


#define Pi32    3.141592653589793
#define Pi32_2  1.570796326794897
#define Pi32_4  0.785398163397448
#define Tau32   6.283185307179586

//
// Macros and "macro-like" utility functions
inline u8 Max(u8 x, u8 y) {return x > y ? x : y;}
inline u8 Min(u8 x, u8 y) {return x < y ? x : y;}



//
// Scalars
inline f32 Square(f32 const& x) {
    f32 Result = x * x;
    return Result;
}

inline f32 SquareRoot(f32 const& x) {
    f32 Result = sqrtf(x);
    return Result;
}

inline f32 Cos(f32 const& a) {
    f32 Result = cosf(a);
    return Result;
}

inline f32 Sin(f32 const& a) {
    f32 Result = sinf(a);
    return Result;
}

inline f32 Tan(f32 const& a) {
    f32 Result = tanf(a);
    return Result;
}

inline f32 Arctan2(f32 const& x, f32 const& y) {
    f32 Result = (f32)atan2(y, x);
    return Result;
}



//
// Vectors
//

//
// v2
#define v2_zero V2(0.0f, 0.0f)
#define v2_one  V2(1.0f, 1.0f)

union v2 {
    struct {
        f32 x;
        f32 y;
    };
    f32 E[2];
};

inline v2 V2(f32 x, f32 y) {
    v2 Result;
    Result.x = x;
    Result.y = y;
    return Result;
}

//
// v2 vs v2
inline v2 operator + (v2 const& A, v2 const& B) {
    v2 Result = V2(A.x + B.x, A.y + B.y);
    return Result;
}

inline v2 operator - (v2 const& A, v2 const& B) {
    v2 Result = V2(A.x - B.x, A.y - B.y);
    return Result;
}

inline v2 Hadamard(v2 const& A, v2 const& B) {
    v2 Result = V2(A.x * B.x, A.y * B.y);
    return Result;
}

inline f32 Dot(v2 const& A, v2 const& B) {
    f32 Result = A.x * B.x + A.y * B.y;
    return Result;
}

//
// v2 vs f32
inline v2 operator * (v2 const& A, float const& b) {
    v2 Result = V2(A.x * b, A.y * b);
    return Result;
}

inline v2 operator * (float const& b, v2 const& A) {
    v2 Result = A * b;
    return Result;
}

//
// v2 unary
inline v2 operator - (v2 const& A) {
    v2 Result = V2(-A.x, -A.y);
    return Result;
}

inline v2 Perp(v2 const& A) {
    v2 Result = V2(-A.y, A.x);
    return Result;
}

inline f32 LengthSq(v2 const& A) {
    f32 Result = A.x * A.x + A.y * A.y;
    return Result;
}

inline f32 Length(v2 const& A) {
    f32 Result = SquareRoot(A.x * A.x + A.y * A.y);
    return Result;
}

inline v2 Normalize(v2 const& A) {
    v2 Result = A * (1.0f / Length(A));
    return Result;
}

// Normalize or zero
inline v2 NOZ(v2 const& A) {
    v2 Result = {};
    
    f32 l = LengthSq(A);
    if(l > Square(0.0001f)) {
        Result = A * (1.0f / SquareRoot(l));
    }
    
    return Result;
}


//
// Line segments
union line_segment {
    struct {
        v2 P0;
        v2 P1;
    };
    v2 P[2];
};

inline line_segment Line_segment(v2 const& A, v2 const& B) {
    line_segment Result;
    Result.P0 = A;
    Result.P1 = B;
    
    return Result;
}

inline line_segment Line_segment(f32 x0, f32 y0, f32 x1, f32 y1) {
    line_segment Result;
    Result.P0 = V2(x0, y0);
    Result.P1 = V2(x1, y1);
    return Result;
}



//
// v3
#define v3_zero V2(0.0f, 0.0f, 0.0f)
#define v3_one  V2(1.0f, 1.0f, 1.0f)

union v3 {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    f32 E[3];
};

inline v3 V3(f32 x, f32 y, f32 z) {
    v3 Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    return Result;
}

//
// v3 vs v3
inline v3 operator + (v3 const& A, v3 const& B) {
    v3 Result = V3(A.x + B.x, A.y + B.y, A.z + B.z);
    return Result;
}

inline v3 operator - (v3 const& A, v3 const& B) {
    v3 Result = V3(A.x - B.x, A.y - B.y, A.z - B.z);
    return Result;
}

inline v3 Hadamard(v3 const& A, v3 const& B) {
    v3 Result = V3(A.x * B.x, A.y * B.y, A.z * B.z);
    return Result;
}

inline f32 Dot(v3 const& A, v3 const& B) {
    f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return Result;
}

inline v3 Cross(v3 const& A, v3 const& B) {
    v3 Result = V3(A.y * B.z - A.z * B.y, 
                   A.z * B.x - A.x * B.z,
                   A.x * B.y - A.y * B.x);
    return Result;
}

//
// v3 vs f32
inline v3 operator * (v3 const& A, float const& b) {
    v3 Result = V3(A.x * b, A.y * b, A.z * b);
    return Result;
}

inline v3 operator * (float const& b, v3 const& A) {
    v3 Result = A * b;
    return Result;
}

//
// v3 unary
inline v3 operator - (v3 const& A) {
    v3 Result = V3(-A.x, -A.y, -A.z);
    return Result;
}

inline f32 LengthSq(v3 const& A) {
    f32 Result = A.x * A.x + A.y * A.y + A.z * A.z;
    return Result;
}

inline f32 Length(v3 const& A) {
    f32 Result = SquareRoot(A.x * A.x + A.y * A.y + A.z * A.z);
    return Result;
}

inline v3 Normalize(v3 const& A) {
    v3 Result = A * (1.0f / Length(A));
    return Result;
}

// Normalize or zero
inline v3 NOZ(v3 const& A) {
    v3 Result = {};
    
    f32 l = LengthSq(A);
    if(l > Square(0.0001f)) {
        Result = A * (1.0f / SquareRoot(l));
    }
    
    return Result;
}


#endif /* Mathematics_h */
