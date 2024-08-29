#pragma once
#include "commons.cpp"

class v2f
{
  public:
	f32 x = 0.0f;
	f32 y = 0.0f;

	v2f();
	v2f(f32 size);
	v2f(f32 x, f32 y);
	v2f(const Vector2& v);
	f32			getLength() const;
	f32			getLengthSquared() const;
	v2f			rotate(f32 angle) const;
	f32			dot(const v2f& obj) const;
	f32			cross(const v2f& obj) const;
	v2f			proj(v2f onTo) const;
	v2f			norm() const;
	v2f			squared() const;
	bool		isZero() const;
	f32			distTo(const v2f& a) const;
	f32			distToSquared(const v2f& a) const;
	static f32	dot(const v2f& a, const v2f& b);
	static f32	dist(const v2f& a, const v2f& b);
	static f32	distSquared(const v2f& a, const v2f& b);
	static v2f	random(f32 range);
	static bool isHeadingTowards(const v2f& aPos, const v2f& aVel, const v2f& bPos);

	Vector2				 toVector2() const;
	v2f					 operator+(v2f const& obj) const;
	v2f					 operator-(v2f const& obj) const;
	v2f					 operator*(v2f const& obj) const;
	v2f					 operator*(f32 obj) const;
	v2f					 operator/(v2f const& obj) const;
	v2f					 operator/(f32 obj) const;
	v2f					 operator-() const;
	bool				 operator==(const v2f& obj) const;
	friend std::ostream& operator<<(std::ostream& os, const v2f& obj);
};
v2f	 operator*(const f32 lhs, const v2f& rhs);
v2f& operator+=(v2f& lhs, v2f const& rhs);
v2f& operator-=(v2f& lhs, v2f const& rhs);
v2f& operator*=(v2f& lhs, v2f const& rhs);
v2f& operator*=(v2f& lhs, f32 rhs);

v2f::v2f()
{
	x = 0.0f;
	y = 0.0f;
}
v2f::v2f(f32 size)
{
	x = size;
	y = size;
}
v2f::v2f(float _x, float _y)
{
	x = _x;
	y = _y;
}
v2f::v2f(const Vector2& v)
{
	x = v.x;
	y = v.y;
}
float v2f::getLength() const { return sqrtf(getLengthSquared()); }
float v2f::getLengthSquared() const { return x * x + y * y; }
v2f	  v2f::rotate(float angle) const
{
	v2f	  rotated;
	float c	  = cosf(angle);
	float s	  = sinf(angle);
	rotated.x = x * c - y * s;
	rotated.y = x * s + y * c;
	return rotated;
}
float v2f::dot(const v2f& v) const { return x * v.x + y * v.y; }
float v2f::cross(const v2f& v) const { return x * v.y - y * v.x; }
v2f	  v2f::proj(v2f onTo) const
{
	float k = dot(onTo) / onTo.dot(onTo);
	return onTo * k;
}
v2f v2f::norm() const
{
	if (isZero())
		return {0, 0};
	float len = getLength();
	return {x / len, y / len};
}
v2f	 v2f::squared() const { return {x * x, y * x}; }
bool v2f::isZero() const
{
	if (fabsf(x) < 0.0001 && fabsf(y) < 0.0001f)
		return true;
	return false;
}
float v2f::distTo(const v2f& a) const { return dist(*this, a); }
float v2f::distToSquared(const v2f& a) const { return distSquared(*this, a); }
float v2f::dot(const v2f& a, const v2f& b) { return a.x * b.x + a.y * b.y; }
float v2f::dist(const v2f& a, const v2f& b) { return (b - a).getLength(); }
float v2f::distSquared(const v2f& a, const v2f& b) { return (b - a).getLengthSquared(); }
v2f	  v2f::random(float range)
{
	return v2f(math::randomf(-range, range), math::randomf(-range, range));
}
bool v2f::isHeadingTowards(const v2f& aPos, const v2f& aVel, const v2f& bPos)
{
	v2f dir = bPos - aPos;
	return dot(dir, aVel) > 0.f;
}

Vector2 v2f::toVector2() const { return {x, y}; }
v2f		v2f::operator+(v2f const& obj) const { return v2f(x + obj.x, y + obj.y); }
v2f		v2f::operator-(v2f const& obj) const { return v2f(x - obj.x, y - obj.y); }
v2f		v2f::operator*(v2f const& obj) const { return v2f(x * obj.x, y * obj.y); }
v2f		v2f::operator*(float obj) const { return v2f(x * obj, y * obj); }
v2f		v2f::operator/(v2f const& obj) const { return v2f(x / obj.x, y / obj.y); }
v2f		v2f::operator/(float obj) const { return v2f(x / obj, y / obj); }
v2f		v2f::operator-() const { return v2f(-x, -y); }
bool	v2f::operator==(const v2f& obj) const { return (x == obj.x) & (y == obj.y); }

std::ostream& operator<<(std::ostream& os, const v2f& obj) { return os << obj.x << ", " << obj.y; }

v2f	 operator*(const float lhs, const v2f& rhs) { return rhs * lhs; }
v2f& operator+=(v2f& lhs, const v2f& rhs) { return lhs = lhs + rhs; }
v2f& operator-=(v2f& lhs, const v2f& rhs) { return lhs = lhs - rhs; }
v2f& operator*=(v2f& lhs, const v2f& rhs) { return lhs = rhs * lhs; }
v2f& operator*=(v2f& lhs, float rhs) { return lhs = rhs * lhs; }
v2f& operator/=(v2f& lhs, const v2f& rhs) { return lhs = lhs / rhs; }
v2f& operator/=(v2f& lhs, float rhs) { return lhs = lhs / rhs; }
bool operator!=(const v2f& lhs, const v2f& rhs) { return !(lhs == rhs); }

namespace math
{
	// line from A to B, point P
	v2f projectPointOntoLine(const v2f& p, const v2f& a, const v2f& b)
	{
		v2f	  ab = b - a;
		v2f	  ap = p - a;
		float t	 = ap.dot(ab) / ab.dot(ab);
		if (t > 1. || t < 0.f)
			return a;
		return a + ab * t;	// The projected point on the line
	}
}  // namespace math

struct v3i
{
	s32 x, y, z;
	v3i() { x = y = z = 0; }
	v3i(s32 xx, s32 yy, s32 zz)
	{
		x = xx;
		y = yy;
		z = zz;
	}
	Vector2 toVector2() const { return {(f32)x, (f32)y}; };
	v3i		operator+(v3i const& obj) const { return v3i(x + obj.x, y + obj.y, z + obj.z); };
	v3i		operator-(v3i const& obj) const { return v3i(x - obj.x, y - obj.y, z - obj.z); };
	v3i		operator*(v3i const& obj) const { return v3i(x * obj.x, y * obj.y, z * obj.z); };
	v3i		operator*(f32 obj) const { return v3i(x * obj, y * obj, z * obj); };
	v3i		operator/(v3i const& obj) const { return v3i(x / obj.x, y / obj.y, z / obj.z); };
	v3i		operator/(f32 obj) const { return v3i(x / obj, y / obj, z / obj); };
	bool	operator==(const v3i& obj) const { return (x == obj.x && y == obj.y && z == obj.z); };
	bool	operator!=(const v3i& obj) const { return !(*this == obj); };
};
v3i	 operator*(f32 lhs, const v3i& rhs) { return rhs * lhs; };
v3i& operator+=(v3i& lhs, const v3i& rhs) { return lhs = lhs + rhs; }
v3i& operator-=(v3i& lhs, const v3i& rhs) { return lhs = lhs - rhs; }
v3i& operator*=(v3i& lhs, const v3i& rhs) { return lhs = rhs * lhs; }
v3i& operator/=(v3i& lhs, const v3i& rhs) { return lhs = lhs / rhs; }

v2f toV2f(const v3i& v) { return {(f32)v.x, (f32)v.y}; }
v3i toV3i(const v2f& v) { return {(s32)v.x, (s32)v.y, 0}; }
