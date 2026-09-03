#pragma once

#include <cmath>

namespace avernal::render {

/// 4x4 matrix for transformations
struct Matrix4x4 {
    float m[16];  // Column-major

    static Matrix4x4 identity() {
        return Matrix4x4{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    static Matrix4x4 ortho(
        float left, float right, float bottom, float top, float near_plane, float far_plane) {
        Matrix4x4 result{};
        result.m[0] = 2.0f / (right - left);
        result.m[5] = 2.0f / (top - bottom);
        result.m[10] = 1.0f / (far_plane - near_plane);
        result.m[12] = -(right + left) / (right - left);
        result.m[13] = -(top + bottom) / (top - bottom);
        result.m[14] = -near_plane / (far_plane - near_plane);
        result.m[15] = 1.0f;
        return result;
    }

    static Matrix4x4 perspective(float fov_y, float aspect, float near_plane, float far_plane) {
        const float tan_half_fov = std::tan(fov_y / 2.0f);
        Matrix4x4 result{};
        result.m[0] = 1.0f / (aspect * tan_half_fov);
        result.m[5] = 1.0f / tan_half_fov;
        result.m[10] = far_plane / (far_plane - near_plane);
        result.m[11] = 1.0f;
        result.m[14] = -(far_plane * near_plane) / (far_plane - near_plane);
        return result;
    }

    static Matrix4x4 look_at(float eye_x, float eye_y, float eye_z,
                             float center_x, float center_y, float center_z,
                             float up_x, float up_y, float up_z) {
        // Left-handed: view +Z points from the eye toward the target so it matches
        // perspective() (D3D-style clip Z in [0, 1], m[11] = 1).
        float fx = center_x - eye_x;
        float fy = center_y - eye_y;
        float fz = center_z - eye_z;
        float f_len = std::sqrt(fx * fx + fy * fy + fz * fz);
        fx /= f_len;
        fy /= f_len;
        fz /= f_len;

        // Right = up × forward
        float rx = up_y * fz - up_z * fy;
        float ry = up_z * fx - up_x * fz;
        float rz = up_x * fy - up_y * fx;
        float r_len = std::sqrt(rx * rx + ry * ry + rz * rz);
        rx /= r_len;
        ry /= r_len;
        rz /= r_len;

        // Up = forward × right
        float ux = fy * rz - fz * ry;
        float uy = fz * rx - fx * rz;
        float uz = fx * ry - fy * rx;

        Matrix4x4 result{};
        result.m[0] = rx;
        result.m[1] = ux;
        result.m[2] = fx;
        result.m[4] = ry;
        result.m[5] = uy;
        result.m[6] = fy;
        result.m[8] = rz;
        result.m[9] = uz;
        result.m[10] = fz;
        result.m[12] = -(rx * eye_x + ry * eye_y + rz * eye_z);
        result.m[13] = -(ux * eye_x + uy * eye_y + uz * eye_z);
        result.m[14] = -(fx * eye_x + fy * eye_y + fz * eye_z);
        result.m[15] = 1.0f;
        return result;
    }

    static Matrix4x4 multiply(const Matrix4x4& a, const Matrix4x4& b) {
        Matrix4x4 result{};
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                }
                result.m[col * 4 + row] = sum;
            }
        }
        return result;
    }
};

/// Camera represents the view and projection for rendering
class Camera {
public:
    Camera() = default;
    ~Camera() = default;

    /// Set up a perspective projection camera
    void set_perspective(float fov_y, float aspect, float near_plane, float far_plane) {
        projection_ = Matrix4x4::perspective(fov_y, aspect, near_plane, far_plane);
    }

    /// Set the camera view (look at)
    void set_look_at(float eye_x, float eye_y, float eye_z,
                     float center_x, float center_y, float center_z,
                     float up_x, float up_y, float up_z) {
        view_ = Matrix4x4::look_at(eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z);
    }

    /// Get the view matrix
    [[nodiscard]] const Matrix4x4& view() const noexcept { return view_; }

    /// Get the projection matrix
    [[nodiscard]] const Matrix4x4& projection() const noexcept { return projection_; }

    /// Get the view-projection matrix
    [[nodiscard]] Matrix4x4 view_projection() const {
        return Matrix4x4::multiply(projection_, view_);
    }

private:
    Matrix4x4 view_ = Matrix4x4::identity();
    Matrix4x4 projection_ = Matrix4x4::identity();
};

}  // namespace avernal::render
