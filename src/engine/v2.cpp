#pragma once

#include "engine.cpp"
#include <cmath>
#include <stdlib.h>
#include <iostream>

namespace math
{
	constexpr f32 pi  = M_PIf;
	constexpr f32 pi2 = pi / 2.f;
	constexpr f32 pi4 = pi / 4.f;
	constexpr f32 tau = 2.f * M_PIf;
	inline f32	  radToDeg(f32 value) { return value * 180.f / pi; };
	f32			  sq(f32 value) { return value * value; }
	float		  limit(float value, float min, float max)
	{
		if (value > max)
			return max;
		if (value < min)
			return min;
		return value;
	}
	float limit(float value, float minmax) { return limit(value, -minmax, minmax); }
	float random(float min, float max)
	{
		float range = fabsf(max - min);
		return (fmodf(rand(), range * 1000.f) / 1000.f + (min < max ? min : max));
	}
}  // namespace math

class v2
{
  public:
	f32 x = 0.0f;
	f32 y = 0.0f;

	v2();
	v2(f32 size);
	v2(f32 x, f32 y);
	v2(const Vector2& v);
	f32			getLength() const;
	f32			getLengthSquared() const;
	v2			rotate(f32 angle) const;
	f32			dot(const v2& obj) const;
	f32			cross(const v2& obj) const;
	v2			proj(v2 onTo) const;
	v2			norm() const;
	v2			squared() const;
	bool		isZero() const;
	f32			distTo(const v2& a) const;
	f32			distToSquared(const v2& a) const;
	static f32	dot(const v2& a, const v2& b);
	static f32	dist(const v2& a, const v2& b);
	static f32	distSquared(const v2& a, const v2& b);
	static v2	random(f32 range);
	static bool isHeadingTowards(const v2& aPos, const v2& aVel, const v2& bPos);

	Vector2				 toVector2() const;
	v2					 operator+(v2 const& obj) const;
	v2					 operator-(v2 const& obj) const;
	v2					 operator*(v2 const& obj) const;
	v2					 operator*(f32 obj) const;
	v2					 operator/(v2 const& obj) const;
	v2					 operator/(f32 obj) const;
	v2					 operator-() const;
	bool				 operator==(const v2& obj) const;
	friend std::ostream& operator<<(std::ostream& os, const v2& obj);
};
v2	operator*(const f32 lhs, const v2& rhs);
v2& operator+=(v2& lhs, v2 const& rhs);
v2& operator-=(v2& lhs, v2 const& rhs);
v2& operator*=(v2& lhs, v2 const& rhs);
v2& operator*=(v2& lhs, f32 rhs);

v2::v2()
{
	x = 0.0f;
	y = 0.0f;
}
v2::v2(f32 size)
{
	x = size;
	y = size;
}
v2::v2(float _x, float _y)
{
	x = _x;
	y = _y;
}
v2::v2(const Vector2& v)
{
	x = v.x;
	y = v.y;
}
float v2::getLength() const { return sqrtf(getLengthSquared()); }
float v2::getLengthSquared() const { return x * x + y * y; }
v2	  v2::rotate(float angle) const
{
	v2	  rotated;
	float c	  = cosf(angle);
	float s	  = sinf(angle);
	rotated.x = x * c - y * s;
	rotated.y = x * s + y * c;
	return rotated;
}
float v2::dot(const v2& v) const { return x * v.x + y * v.y; }
float v2::cross(const v2& v) const { return x * v.y - y * v.x; }
v2	  v2::proj(v2 onTo) const
{
	float k = dot(onTo) / onTo.dot(onTo);
	return onTo * k;
}
v2 v2::norm() const
{
	if (isZero())
		return {0, 0};
	float len = getLength();
	return {x / len, y / len};
}
v2	 v2::squared() const { return {x * x, y * x}; }
bool v2::isZero() const
{
	if (fabsf(x) < 0.0001 && fabsf(y) < 0.0001f)
		return true;
	return false;
}
float v2::distTo(const v2& a) const { return dist(*this, a); }
float v2::distToSquared(const v2& a) const { return distSquared(*this, a); }
float v2::dot(const v2& a, const v2& b) { return a.x * b.x + a.y * b.y; }
float v2::dist(const v2& a, const v2& b) { return (b - a).getLength(); }
float v2::distSquared(const v2& a, const v2& b) { return (b - a).getLengthSquared(); }
v2 v2::random(float range) { return v2(math::random(-range, range), math::random(-range, range)); }
bool v2::isHeadingTowards(const v2& aPos, const v2& aVel, const v2& bPos)
{
	v2 dir = bPos - aPos;
	return dot(dir, aVel) > 0.f;
}

Vector2 v2::toVector2() const { return {x, y}; }
v2		v2::operator+(v2 const& obj) const { return v2(x + obj.x, y + obj.y); }
v2		v2::operator-(v2 const& obj) const { return v2(x - obj.x, y - obj.y); }
v2		v2::operator*(v2 const& obj) const { return v2(x * obj.x, y * obj.y); }
v2		v2::operator*(float obj) const { return v2(x * obj, y * obj); }
v2		v2::operator/(v2 const& obj) const { return v2(x / obj.x, y / obj.y); }
v2		v2::operator/(float obj) const { return v2(x / obj, y / obj); }
v2		v2::operator-() const { return v2(-x, -y); }
bool	v2::operator==(const v2& obj) const { return (x == obj.x) & (y == obj.y); }

std::ostream& operator<<(std::ostream& os, const v2& obj) { return os << obj.x << ", " << obj.y; }

v2	operator*(const float lhs, const v2& rhs) { return rhs * lhs; }
v2& operator+=(v2& lhs, const v2& rhs) { return lhs = lhs + rhs; }
v2& operator-=(v2& lhs, const v2& rhs) { return lhs = rhs - lhs; }
v2& operator*=(v2& lhs, const v2& rhs) { return lhs = rhs * lhs; }
v2& operator*=(v2& lhs, float rhs) { return lhs = rhs * lhs; }
