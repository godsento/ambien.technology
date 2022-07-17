#pragma once
#include "vector2.h"

#define M_PI		(float)3.14159265358979323846f
#define M_RADPI		57.295779513082f
#define M_PIRAD     0.01745329251f
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI) )
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI / 180.f) )

namespace math {
    // pi constants.
    constexpr float pi = 3.1415926535897932384f; // pi
    constexpr float pi_2 = pi * 2.f;               // pi * 2

    // degrees to radians.
    __forceinline constexpr float deg_to_rad(float val) {
        return val * (pi / 180.f);
    }

    // radians to degrees.
    __forceinline constexpr float rad_to_deg(float val) {
        return val * (180.f / pi);
    }

    // angle mod ( shitty normalize ).
    __forceinline float AngleMod(float angle) {
        return (360.f / 65536) * ((int)(angle * (65536.f / 360.f)) & 65535);
    }

    void AngleMatrix(const ang_t& ang, const vec3_t& pos, matrix3x4_t& out);

    // normalizes an angle.
    void NormalizeAngle(float& angle);

    __forceinline float NormalizedAngle(float angle) {
        NormalizeAngle(angle);
        return angle;
    }

    typedef __declspec(align(16)) union {
        float f[4];
        __m128 v;
    } m128;
    inline __m128 sqrt_ps(const __m128 squared) {
        return _mm_sqrt_ps(squared);
    }

    inline float lerp(float percent, float a, float b) {

        return a + (b - a) * percent;

    };

    inline float simple_spline(float value) {

        float squared_value = value * value;

        return (3 * squared_value - 2 * squared_value * value);

    }

    inline float simple_spline_remap_val_clamped(float val, float a, float b, float c, float d) {

        if (a == b)
            return val >= b ? d : c;

        float clamped_value = (val - a) / (b - a);
        clamped_value = std::clamp(clamped_value, 0.f, 1.f);
        return c + (d - c) * simple_spline(clamped_value);

    }
    inline float AngleDiff(float src, float dst) {
        float delta;

        delta = fmodf(dst - src, 360.0f);
        if (dst > src)
        {
            if (delta >= 180)
                delta -= 360;
        }
        else
        {
            if (delta <= -180)
                delta += 360;
        }
        return delta;
    }
    float ApproachAngle(float target, float value, float speed);
    void  VectorAngles(const vec3_t& forward, ang_t& angles, vec3_t* up = nullptr);
    void  AngleVectors(const ang_t& angles, vec3_t* forward, vec3_t* right = nullptr, vec3_t* up = nullptr);
    vec3_t vector_angles(const vec3_t& v);
    vec3_t angle_vectors(const vec3_t& angles);
    float GetFOV(const ang_t& view_angles, const vec3_t& start, const vec3_t& end);
    void inline SinCos(float radians, float* sine, float* cosine);
    void angle_matrix(const ang_t& angles, const vec3_t& position, matrix3x4_t& matrix);
    void matrix_set_column(const vec3_t& in, int column, matrix3x4_t& out);
    void angle_matrix(const ang_t& angles, matrix3x4_t& matrix);
    vec3_t vector_rotate(const vec3_t& in1, const matrix3x4_t& in2);
    vec3_t vector_rotate(const vec3_t& in1, const ang_t& in2);
    void vector_i_rotate(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out);
    void  VectorTransform(const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out);
    bool  IntersectLineWithBB(vec3_t& vStart, vec3_t& vEndDelta, vec3_t& vMin, vec3_t& vMax);
    void  VectorITransform(const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out);
    float SegmentToSegment(const vec3_t s1, const vec3_t s2, const vec3_t k1, const vec3_t k2);
    void  MatrixAngles(const matrix3x4_t& matrix, ang_t& angles);
    void  MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out);
    void  ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out);

    vec3_t vector_transform(vec3_t in, matrix3x4_t matrix);

    vec3_t CalcAngle(const vec3_t& vecSource, const vec3_t& vecDestination);

    // computes the intersection of a ray with a box ( AABB ).
    bool IntersectRayWithBox(const vec3_t& start, const vec3_t& delta, const vec3_t& mins, const vec3_t& maxs, float tolerance, BoxTraceInfo_t* out_info);
    bool IntersectRayWithBox(const vec3_t& start, const vec3_t& delta, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr, float* fraction_left_solid = nullptr);

    // computes the intersection of a ray with a oriented box ( OBB ).
    bool IntersectRayWithOBB(const vec3_t& start, const vec3_t& delta, const matrix3x4_t& obb_to_world, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr);
    bool IntersectRayWithOBB(const vec3_t& start, const vec3_t& delta, const vec3_t& box_origin, const ang_t& box_rotation, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr);

    // returns whether or not there was an intersection of a sphere against an infinitely extending ray. 
    // returns the two intersection points.
    bool IntersectInfiniteRayWithSphere(const vec3_t& start, const vec3_t& delta, const vec3_t& sphere_center, float radius, float* out_t1, float* out_t2);

    // returns whether or not there was an intersection, also returns the two intersection points ( clamped 0.f to 1.f. ).
    // note: the point of closest approach can be found at the average t value.
    bool IntersectRayWithSphere(const vec3_t& start, const vec3_t& delta, const vec3_t& sphere_center, float radius, float* out_t1, float* out_t2);

    vec3_t Interpolate(const vec3_t from, const vec3_t to, const float percent);

    void GetRadius2d(std::vector<vec2_t>* input, vec3_t pos, int radius, int points);

    float SignedArea(vec2_t p, vec2_t q, vec2_t r);

    bool GiftWrap(std::vector<vec2_t> input, std::vector<vec2_t>* output);

    template < typename t >
    __forceinline void clamp(t& n, const t& lower, const t& upper) {
        n = std::max(lower, std::min(n, upper));
    }
}