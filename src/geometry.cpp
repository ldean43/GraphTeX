#include "geometry.hpp"

void Geometry::generateVertices() {
    // truncate small decimals
    const float epsilon = 1e-6;
    for (int i = 0; i < step_; i++) {
        float y = -range_ + i * step_size_;
        for (int j = 0; j < step_; j++) {
            float x = -range_ + j * step_size_;
            x = std::abs(x) < epsilon ? 0.0 : x;
            y = std::abs(y) < epsilon ? 0.0 : y;
            evaluator_->vars_["x"] = x;
            evaluator_->vars_["y"] = y;
            int index = 3 * (i * step_ + j);
            vertices_[index] = x;
            vertices_[index + 1] = y;
            vertices_[index + 2] = evaluator_->evaluate();
        }
    }
    evaluator_->vars_.erase("x");
    evaluator_->vars_.erase("y");
}

void Geometry::generateNormals() {
    std::vector<vec3> normals(vertices_.size()/3);
    for (size_t i = 0; i < indices_.size() - 2; i++) {
        int i0 = indices_[i];
        int i1 = indices_[i + 1];
        int i2 = indices_[i + 2];

        // skipping degenerate triangles
        if (i0 == i1 || i0 == i2 || i1 == i2) {
            continue;
        }

        vec3 v0(vertices_[i0 * 3], vertices_[i0 * 3 + 1], vertices_[i0 * 3 + 2]);
        vec3 v1(vertices_[i1 * 3], vertices_[i1 * 3 + 1], vertices_[i1 * 3 + 2]);
        vec3 v2(vertices_[i2 * 3], vertices_[i2 * 3 + 1], vertices_[i2 * 3 + 2]);

        // Triangle strip swaps vertex order for winding
        vec3 normal = (i % 2 == 0) ? 
            (v1 - v0).cross(v2 - v0) :
            (v2 - v0).cross(v1 - v0);
        normal = normal.normalize();

        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }

    for (size_t i = 0; i < normals.size(); i++) {
        normals_.push_back(normals[i].normalize().x);
        normals_.push_back(normals[i].normalize().y);
        normals_.push_back(normals[i].normalize().z);
    }
}

// Reconstructs triangles if clips through max/min z plane
void Geometry::clipTriangles() {
    std::vector<float> new_vertices;
    vec3 v0, v1, v2;
    uint32_t i0, i1, i2;
    float bound = range_;
    for (size_t row = 1; row < step_; row++) {
        for (size_t col = 0; col < step_; col ++) {
            if (col % 2) {
                i0 = (step_ * row + col) * 3;
                i1 = (step_ * (row - 1) + (col - 1)) * 3;
                i2 = (step_ * (row - 1) + col) * 3;
                v0 = vec3(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
                v1 = vec3(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
                v2 = vec3(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
            } else {
                i0 = (step_ * row + col) * 3;
                i1 = (step_ * (row - 1) + col) * 3;
                i2 = (step_ * row + col + 1) * 3;
                v0 = vec3(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
                v1 = vec3(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
                v2 = vec3(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
            }
            if (!crossDiscontinuity(v0, v1, v2)) {
                // Detect above range_
                bool v0_out, v1_out, v2_out;
                bool v0_out_above = v0.z > range_;
                bool v1_out_above = v1.z > range_;
                bool v2_out_above = v2.z > range_;

                bool v0_out_below = v0.z < range_;
                bool v1_out_below = v1.z < range_;
                bool v2_out_below = v2.z < range_;
                
                unsigned int out_above = v0_out_above + v1_out_above + v2_out_above;
                unsigned int out_below = v0_out_below + v1_out_below + v2_out_below;
                unsigned int out = std::max(out_above, out_below);
                if (out_above > out_below) {
                    bound = range_;
                    v0_out = v0_out_above;
                    v1_out = v1_out_above;
                    v2_out = v2_out_above;
                } else {
                    bound = -range_;
                    v0_out = v0_out_below;
                    v1_out = v1_out_below;
                    v2_out = v2_out_below;
                }

                float t = 0;
                if (out == 2) {
                    if (v0_out && v1_out) {
                        t = (bound - v2.z)/(v0.z - v2.z);
                        vec3 v3 = v0*t + v2*(1-t);
                        t = (bound - v2.z)/(v1.z - v2.z);
                        vec3 v4 = v1*t + v2*(1-t);
                        pushVertex(v3, v4, v2, new_vertices);
                    } else if (v0_out && v2_out) {
                        t = (bound - v1.z)/(v0.z - v1.z);
                        vec3 v3 = v0*t + v1*(1-t);
                        t = (bound - v1.z)/(v2.z - v1.z);
                        vec3 v4 = v2*t + v1*(1-t);
                        pushVertex(v3, v1, v4, new_vertices);
                    } else {
                        t = (bound - v0.z)/(v1.z - v0.z);
                        vec3 v3 = v1*t + v0*(1-t);
                        t = (bound - v0.z)/(v2.z - v0.z);
                        vec3 v4 = v2*t + v0*(1-t);
                        pushVertex(v0, v3, v4, new_vertices);
                    }
                } else if (out == 1) {
                    if (v0_out) {
                        t = (bound - v1.z)/(v0.z - v1.z);
                        vec3 v3 = v0*t + v1*(1-t);
                        t = (bound - v2.z)/(v0.z - v2.z);
                        vec3 v4 = v0*t + v2*(1-t);
                        pushVertex(v3, v1, v4, new_vertices);
                        pushVertex(v4, v1, v2, new_vertices);
                    } else if (v1_out) {
                        t = (bound - v0.z)/(v1.z - v0.z);
                        vec3 v3 = v1*t + v0*(1-t);
                        t = (bound - v2.z)/(v1.z - v2.z);
                        vec3 v4 = v1*t + v2*(1-t);
                        pushVertex(v0, v3, v2, new_vertices);
                        pushVertex(v2, v3, v4, new_vertices);
                    } else {
                        t = (bound - v0.z)/(v2.z - v0.z);
                        vec3 v3 = v2*t + v0*(1-t);
                        t = (bound - v1.z)/(v2.z - v1.z);
                        vec3 v4 = v2*t + v1*(1-t);
                        pushVertex(v0, v1, v3, new_vertices);
                        pushVertex(v3, v1, v4, new_vertices);
                    }
                }
            }
        }
    }
}

bool Geometry::crossDiscontinuity(vec3 v0, vec3 v1, vec3 v2) {
    bool above = v0.z > range_ || v1.z > range_ || v2.z > range_;
    bool below = v0.z < range_ || v1.z < range_ || v2.z < range_;
    float d0 = abs(v0.z - v1.z);
    float d1 = abs(v0.z - v2.z);
    float d2 = abs(v1.z - v2.z);
    return (above && below) || std::max(d0, std::max(d1, d2)) > range_;
}

void Geometry::pushVertex(vec3 v0, vec3 v1, vec3 v2, std::vector<float>& verts) {
        verts.insert(verts.end(), {v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z});
};
