#pragma once
#include <math.h>

class KVector2
{
public:
    static KVector2 zero;
    static KVector2 one;
    static KVector2 right;
    static KVector2 up;
    static KVector2 Lerp(const KVector2& begin, const KVector2& end, double ratio);

    double  x;
    double  y;

    KVector2(double tx = 0.0, double ty = 0.0) { x = tx; y = ty; }
    double Length() const
    {
        return sqrt(x*x + y*y);
    }
    void Normalize()
    {
        const double length = Length();
        x = x / length;
        y = y / length;
    }
};

inline KVector2 operator+( const KVector2& lhs, const KVector2& rhs )
{
    KVector2 temp( lhs.x + rhs.x, lhs.y + rhs.y );
    return temp;
}

inline KVector2 operator-( const KVector2& lhs, const KVector2& rhs )
{
    KVector2 temp( lhs.x - rhs.x, lhs.y - rhs.y );
    return temp;
}

inline KVector2 operator*(double scalar, const KVector2& rhs)
{
    KVector2 temp(scalar*rhs.x, scalar*rhs.y);
    return temp;
}

inline KVector2 operator*(const KVector2& lhs, double scalar)
{
    KVector2 temp(scalar*lhs.x, scalar*lhs.y);
    return temp;
}
