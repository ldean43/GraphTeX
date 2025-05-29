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
/*
    Generates indices for triangle strip, alternating from left to right 
    and right to left each row
*/
void Geometry::generateIndices() {
    for (int i = 0; i < step_ - 1; i++) {
        if (i % 2) {
            /*
                Degenerate Triangle:
                Since the vertex on the next row will be to the right, index
                is at i*step_ + step_ - 1
            */
            indices_.push_back(i*step_ + step_ - 1);
            // Triangle strip from right to left
            if (i + 1 < step_) {
                for (int j = step_ - 1; j > 0; j--) {
                    indices_.push_back((i+1)*step_ + j);
                    indices_.push_back(i*step_ + j - 1);
                }
            }
        } else {
            /*  
                Degenerate Triangle: 
                Since the vertex on the next row will be to the left, index
                is at i*step_
            */
            if (i != 0) {
                indices_.push_back(i*step_);
            }
            // Triangle strip from left to right
            for (int j = 0; j < step_; j++) {
                indices_.push_back((i+1)*step_ + j);
                indices_.push_back(i*step_ + j);
            }
        }
    }
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