#include "math_util.h"

float Complex::magnitude() const
{
    return sqrtf(real*real+imaginary*imaginary);
}

float Complex::winding() const
{
	return atan2f(imaginary, real);
}


float snap_value(float val, float bound1, float bound2, float threshold)
{
  if (fabsf(val - bound1) <= threshold) {
    val = bound1;
  }
  else if (fabsf(val - bound2) <= threshold) {
    val = bound2;
  }
  else if (fabsf(val - (bound1 + bound2) / 2) <= threshold) {
    val = (bound1 + bound2) / 2;
  }

  return val;
}

Color::Color(const ColorF& color) {
    float coef = 255.0;
    r = int(color.r * coef);
    g = int(color.g * coef);
    b = int(color.b * coef);
    a = int(color.a * coef);
}

ColorF::ColorF(const Color& color) {
    float coef = 1.0 / 255.0;
    r = (float)color.r * coef;
    g = (float)color.g * coef;
    b = (float)color.b * coef;
    a = (float)color.a * coef;
}


float lerp(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

vec2 lerp2(vec2 a, vec2 b, float t)
{
    return vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

vec2 reflect2(vec2 incident, vec2 normal)
{
    return incident - 2.0f * dot2(normal, incident) * normal;
}

vec2 get_direction_vector(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return vec2(c, s);
}

bool Rectangle::contains(vec2 p) const
{
    return p.x >= x && p.y >= y && p.x <= x + w && p.y <= y + h;
}

