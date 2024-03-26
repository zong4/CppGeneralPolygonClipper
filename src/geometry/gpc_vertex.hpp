#pragma once

#include <iostream>
#include <sstream>

namespace gpc {

// Polygon vertex structure
class gpc_vertex
{
public:
    double x = 0.0; // Vertex x component
    double y = 0.0; // vertex y component

public:
    gpc_vertex() = default;
    gpc_vertex(double in_x, double in_y) : x(in_x), y(in_y) {}
    ~gpc_vertex() = default;

    inline bool operator==(const gpc_vertex &rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }
    inline bool operator!=(const gpc_vertex &rhs) const
    {
        return !(*this == rhs);
    }
    inline gpc_vertex operator+(const gpc_vertex &rhs) const
    {
        return gpc_vertex(x + rhs.x, y + rhs.y);
    }
    inline gpc_vertex operator+=(const gpc_vertex &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    inline gpc_vertex operator-(const gpc_vertex &rhs) const
    {
        return gpc_vertex(x - rhs.x, y - rhs.y);
    }
    inline gpc_vertex operator-=(const gpc_vertex &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    inline gpc_vertex operator*(double scalar) const
    {
        return gpc_vertex(x * scalar, y * scalar);
    }
    inline gpc_vertex operator*=(double scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    inline double operator*(const gpc_vertex &rhs) const
    {
        return x * rhs.x + y * rhs.y;
    }

    friend std::istream &operator>>(std::istream &is, gpc_vertex &vertex)
    {
        is >> vertex.x >> vertex.y;
        return is;
    }

    friend std::ostream &operator<<(std::ostream &os, const gpc_vertex &vertex)
    {
        os << vertex.x << " " << vertex.y;
        return os;
    }

    inline void rotate(double angle)
    {
        double x_new = x * std::cos(angle) - y * std::sin(angle);
        double y_new = x * std::sin(angle) + y * std::cos(angle);
        x = x_new;
        y = y_new;
    }

    inline void rotate90()
    {
        double x_new = -y;
        double y_new = x;
        x = x_new;
        y = y_new;
    }

    inline void rotate90_reverse()
    {
        double x_new = y;
        double y_new = -x;
        x = x_new;
        y = y_new;
    }

    inline double norm() const { return std::sqrt(x * x + y * y); }

    inline void normalize()
    {
        double length = norm();
        x /= length;
        y /= length;
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "gpc_vertex: (" << x << ", " << y << ")";
        return ss.str();
    }
};

} // namespace gpc