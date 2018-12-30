#include <stdio.h>
#include "Mathematics.h"

void Print(m4& C)
{
    for (int Row = 0; Row < 4; ++Row)
    {
        for (int Col = 0; Col < 4; ++Col)
        {
            printf("%8.4f", C.E[(4 * Col) + Row]);
            if (Col < 3)  printf(", ");
        }
        printf("\n");
    }
}

void PrintMem(m4& C)
{
    for (int i = 0; i < 16; ++i)
    {
        printf("%8.4f", C.E[i]);
        if (i < 15)  printf(", ");
    }
}

void Print(v4& C)
{
    for (int Row = 0; Row < 4; ++Row)
    {
        printf("%10.4f", C.E[Row]);
        if (Row < 3)  printf("\n");
    }
}

v4 DebugMul(m4& A, v4& B)
{
    v4 Result;
    for (int RowIndex = 0; RowIndex < 4; ++RowIndex) 
    {
        v4 R = A.Row(RowIndex);
        f32 D = Dot(R, B);
        
        for (int i = 0; i < 4; ++i)
        {
            printf("%f * %f", R.E[i], B.E[i]);
            if (i < 3)  printf(" + ");
        }
        printf(" = %f\n", D);
        Result.E[RowIndex] = D;
    }
    return Result;
}

int main()
{
#if 0
    m4 A, B;
    for (int i = 0; i < 16; ++i)  A.E[i] = i + 1;
    for (int i = 0; i < 16; ++i)  B.E[i] = 16 - i;
    m4 Result  = A * B;
    
    for (int Col = 0; Col < 4; ++Col)
    {
        for (int Row = 0; Row < 4; ++Row)
        {
            printf("%d%d", Row, Col);
            if (Row < 3)  printf(", ");
        }
        printf("\n");
    }
    
    printf("\nA:\n");
    Print(A);
    
    printf("\nB:\n");
    Print(B);
    
    printf("\nResult:\n");
    Print(&Result);
#endif
    
    v4 Zero   = {   0.0f,    0.0f, 0.0f, 1.0f};
    v4 MaxX   = {1024.0f,    0.0f, 0.0f, 1.0f};
    v4 MaxY   = {   0.0f, 1024.0f, 0.0f, 1.0f};
    v4 Center = { 512.0f,  512.0f, 0.0f, 1.0f};
    
    u32 Width = 1024;
    u32 Height = 1024;
    f32 Far = 1.0f;
    
    f32 Sx = 2.0f / (f32)Width;
    printf("Sx = %f\n", Sx);
    
    f32 Sy = 2.0f / (f32)Height;
    printf("Sy = %f\n", Sy);
    
    f32 Sz = 1.0f / Far;
    printf("Sz = %f\n", Sz);
    
    v4 X = {   Sx,  0.0f, 0.0f, 0.0f};
    v4 Y = { 0.0f,    Sy, 0.0f, 0.0f};
    v4 Z = { 0.0f,  0.0f,   Sz, 0.0f};
    v4 W = {-1.0f, -1.0f, 0.0f, 1.0f};
    m4 M = M4(X, Y, Z, W);
    printf("M:\n");
    PrintMem(M);
    printf("\n");
    
    printf("Zero:\n");
#if 0
    v4 R = M * Zero;
    Print(R);
#else
    v4 R = DebugMul(M, Zero);
#endif
    printf("\n");
    
    printf("MaxX:\n");
    R = M * MaxX;
    Print(R);
    printf("\n");
    
    printf("MaxY:\n");
    R = M * MaxY;
    Print(R);
    printf("\n");
    
    printf("Center:\n");
    R = M * Center;
    Print(R);
    printf("\n");
    
    return 0;
}