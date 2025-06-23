#include "geometry.hpp"

Geometry::Geometry(Evaluator* evaluator, int step, int range, bool clip) {
    evaluator_ = evaluator;
    step_ = step;
    range_ = range;
    step_size_ = 2.0f * range_ / (step_ - 1);
    vertices_.resize(3 * step_ * step_);
    tempVertices_.reserve(3 * step_ * step_);
    normals_.reserve(3 * step_ * step_);
    generateVertices(0, step_);
    clipTriangles(1, step_, clip);
    vertices_ = tempVertices_;
}

void Geometry::generateVertices(int minrow, int maxrow) {
    // truncate small decimals
    const float epsilon = 1e-6;
    Evaluator* localeval = evaluator_->copy();
    for (int i = minrow; i < maxrow; i++) {
        float y = -range_ + i * step_size_;
        for (int j = 0; j < step_; j++) {
            float x = -range_ + j * step_size_;
            x = std::abs(x) < epsilon ? 0.0 : x;
            y = std::abs(y) < epsilon ? 0.0 : y;
            localeval->vars_["x"] = x;
            localeval->vars_["y"] = y;
            int index = 3 * (i * step_ + j);
            // Bound vertices in [-10, 10] WebGL coords
            vertices_[index] = 20*(x + range_)/(2*range_) - 10;
            vertices_[index + 1] = 20*(y + range_)/(2*range_) - 10;
            float z = localeval->evaluate();
            z = std::abs(z) < epsilon ? 0.0 : z;
            vertices_[index + 2] = 20*(z + range_)/(2*range_) - 10;
        }
    }
    delete localeval;
}

// Reconstructs triangles if clips through max/min z plane, along with dynamic normal generaion
void Geometry::clipTriangles(int minrow, int maxrow, bool clip) {
    std::vector<float> new_vertices;
    std::unordered_map<vec3, vec3, vec3::Vec3Hash> normal_map;
    vec3 v0, v1, v2;
    std::vector<vec3> surrounding_grads;
    size_t i0, i1, i2;
    float bound = 10.0f;
    for (int row = minrow; row < maxrow; row++) {
        // For fist col of new row, examine each nearby gradient for prev_grad
        for (int col = 0; col < step_ - 1; col++) { // For each quad
            for (int i = 0; i < 2; i++) { // Two triangles per quad
                surrounding_grads = computeSurroundingGradients(row, col);
                if (i == 0) {
                    i0 = (step_ * row + col) * 3;
                    i1 = (step_ * (row - 1) + col) * 3;
                    i2 = (step_ * row + col + 1) * 3;
                    v0 = vec3(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
                    v1 = vec3(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
                    v2 = vec3(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
                } else {
                    i0 = (step_ * row + (col + 1)) * 3;
                    i1 = (step_ * (row - 1) + col) * 3;
                    i2 = (step_ * (row - 1) + (col + 1)) * 3;
                    v0 = vec3(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
                    v1 = vec3(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
                    v2 = vec3(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
                }
                if (!crossDiscontinuity(v0, v1, v2, surrounding_grads, i % 2)) {
                    if (!clip) { 
                        pushVertex(v0, v1, v2, new_vertices);
                        pushNormal(v0, v1, v2, normal_map);
                        continue;
                    }
                    // Detect above range_
                    bool v0_out, v1_out, v2_out;
                    bool v0_out_above = v0.z > 10.0f;
                    bool v1_out_above = v1.z > 10.0f;
                    bool v2_out_above = v2.z > 10.0f;
    
                    bool v0_out_below = v0.z < -10.0f;
                    bool v1_out_below = v1.z < -10.0f;
                    bool v2_out_below = v2.z < -10.0f;
                    
                    int out_above = v0_out_above + v1_out_above + v2_out_above;
                    int out_below = v0_out_below + v1_out_below + v2_out_below;
                    int out = out_above + out_below > 2 ? 3 : std::max(out_above, out_below);
                    if (out_above >= out_below) {
                        bound = 10.0f;
                        v0_out = v0_out_above;
                        v1_out = v1_out_above;
                        v2_out = v2_out_above;
                    } else {
                        bound = -10.0f;
                        v0_out = v0_out_below;
                        v1_out = v1_out_below;
                        v2_out = v2_out_below;
                    }
                    if (out == 2 || out_above + out_below == 2) {
                        if (v0_out_above && v1_out_below) {
                            vec3 v3 = v2.lerp(v0, (bound - v2.z)/(v0.z - v2.z));
                            vec3 v4 = v2.lerp(v1, (-bound - v2.z)/(v1.z - v2.z));
    
                            pushVertex(v3, v4, v2, new_vertices);
                            pushNormal(v3, v4, v2, normal_map);
                        } else if (v0_out_above && v2_out_below) {
                            vec3 v3 = v1.lerp(v0, (bound - v1.z)/(v0.z - v1.z));
                            vec3 v4 = v1.lerp(v2, (-bound - v1.z)/(v2.z - v1.z));
    
                            pushVertex(v3, v1, v4, new_vertices);
                            pushNormal(v3, v1, v4, normal_map);
                        } else if (v0_out_below && v1_out_above) {
                            vec3 v3 = v2.lerp(v0, (-bound - v2.z)/(v0.z - v2.z));
                            vec3 v4 = v2.lerp(v1, (bound - v2.z)/(v1.z - v2.z));
    
                            pushVertex(v3, v4, v2, new_vertices);
                            pushNormal(v3, v4, v2, normal_map);
                        } else if (v0_out_below && v2_out_above) {
                            vec3 v3 = v1.lerp(v0, (-bound - v1.z)/(v0.z - v1.z));
                            vec3 v4 = v1.lerp(v2, (bound - v1.z)/(v2.z - v1.z));
    
                            pushVertex(v3, v1, v4, new_vertices);
                            pushNormal(v3, v1, v4, normal_map);
                        } else if (v1_out_above && v2_out_below) {
                            vec3 v3 = v0.lerp(v1, (bound - v0.z)/(v1.z - v0.z));
                            vec3 v4 = v0.lerp(v2, (-bound - v0.z)/(v2.z - v0.z));
    
                            pushVertex(v0, v3, v4, new_vertices);
                            pushNormal(v0, v3, v4, normal_map);
                        } else if (v1_out_below && v2_out_above) {
                            vec3 v3 = v0.lerp(v1, (-bound - v0.z)/(v1.z - v0.z));
                            vec3 v4 = v0.lerp(v2, (bound - v0.z)/(v2.z - v0.z));
    
                            pushVertex(v0, v3, v4, new_vertices);
                            pushNormal(v0, v3, v4, normal_map);
                        } else if (v0_out && v1_out) {
                            vec3 v3 = v2.lerp(v0, (bound - v2.z)/(v0.z - v2.z));
                            vec3 v4 = v2.lerp(v1, (bound - v2.z)/(v1.z - v2.z));
    
                            pushVertex(v3, v4, v2, new_vertices);
                            pushNormal(v3, v4, v2, normal_map);
                        } else if (v0_out && v2_out) {
                            vec3 v3 = v1.lerp(v0, (bound - v1.z)/(v0.z - v1.z));
                            vec3 v4 = v1.lerp(v2, (bound - v1.z)/(v2.z - v1.z));
    
                            pushVertex(v3, v1, v4, new_vertices);
                            pushNormal(v3, v1, v4, normal_map);
                        } else {
                            vec3 v3 = v0.lerp(v1, (bound - v0.z)/(v1.z - v0.z));
                            vec3 v4 = v0.lerp(v2, (bound - v0.z)/(v2.z - v0.z));
    
                            pushVertex(v0, v3, v4, new_vertices);
                            pushNormal(v0, v3, v4, normal_map);
                        }
                    } else if (out == 1) {
                        if (v0_out) {
                            vec3 v3 = v1.lerp(v0, (bound - v1.z)/(v0.z - v1.z));
                            vec3 v4 = v2.lerp(v0, (bound - v2.z)/(v0.z - v2.z));
    
                            pushVertex(v3, v1, v4, new_vertices);
                            pushVertex(v4, v1, v2, new_vertices);
                            pushNormal(v3, v1, v4, normal_map);
                            pushNormal(v4, v1, v2, normal_map);
                        } else if (v1_out) {
                            vec3 v3 = v0.lerp(v1, (bound - v0.z)/(v1.z - v0.z));
                            vec3 v4 = v2.lerp(v1, (bound - v2.z)/(v1.z - v2.z));
    
                            pushVertex(v0, v3, v2, new_vertices);
                            pushVertex(v2, v3, v4, new_vertices);
                            pushNormal(v0, v3, v2, normal_map);
                            pushNormal(v2, v3, v4, normal_map);
                        } else {
                            vec3 v3 = v0.lerp(v2, (bound - v0.z)/(v2.z - v0.z));
                            vec3 v4 = v1.lerp(v2, (bound - v1.z)/(v2.z - v1.z));
    
                            pushVertex(v0, v1, v3, new_vertices);
                            pushVertex(v3, v1, v4, new_vertices);
                            pushNormal(v0, v1, v3, normal_map);
                            pushNormal(v3, v1, v4, normal_map);
                        }
                    } else if (out == 0) {
                        pushVertex(v0, v1, v2, new_vertices);
                        pushNormal(v0, v1, v2, normal_map);
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < new_vertices.size(); i += 3) {
        vec3 v(new_vertices[i], new_vertices[i + 1], new_vertices[i + 2]);
        vec3 normal = normal_map[v].normalize();
        normals_.push_back(normal.x);
        normals_.push_back(normal.y);
        normals_.push_back(normal.z);
    }
    tempVertices_.insert(tempVertices_.end(), new_vertices.begin(), new_vertices.end());
}

bool Geometry::crossDiscontinuity(vec3 v0, vec3 v1, vec3 v2, std::vector<vec3> surrounding_grads, bool odd) {
    if (!v0.isfinite() || !v1.isfinite() || !v2.isfinite()) {
        std::cerr << "[crossDiscontinuity] Non-finite triangle vertex\n";
        return true;
    }

    vec3 grad = computeGradient(v0, v1, v2, odd);
    if (!grad.isfinite()) {
        std::cerr << "[crossDiscontinuity] Non-finite gradient computed\n";
        return true;
    }

    vec3 dir1 = vec3(1, 0, 0);
    vec3 dir2 = vec3(0, 1, 0);
    vec3 v0_to_v1 = v1 - v0;
    vec3 v0_to_v2 = v2 - v0;
    vec3 norm = v0_to_v2.cross(v0_to_v1);

    // scale how much derivates need to differ from their surrounding derivates as range changes
    float threshold = 50.0f/(range_ * .25f);

    float dzdx = grad.dot(dir1);
    float dzdy = grad.dot(dir2);

    // checking derivatives w.r.t y
    if (surrounding_grads[0].isfinite() && surrounding_grads[2].isfinite()) {
        float surr_dy1 = surrounding_grads[0].dot(dir2);
        float surr_dy2 = surrounding_grads[2].dot(dir2);

        /* If the surrounding deriv have same sign, and deriv w.r.t y does not, and
        the deriv w.r.t y is near perpindicular to z-axis, and different enough from the it's
        surrounding derivates, its a discontinuity.
        */ 
        if (surr_dy1 * surr_dy2 > 0 && dzdy * surr_dy1 < 0 && 
            norm.dot(vec3(0,0,1)) < 0.01f &&
            std::abs(surr_dy1 - dzdy) > threshold &&
            std::abs(surr_dy2 - dzdy) > threshold) {
            return true;
        }
    } else return true;
    // checking derivatives w.r.t x
    if (surrounding_grads[1].isfinite() && surrounding_grads[3].isfinite()) {
        float surr_dx1 = surrounding_grads[1].dot(dir1);
        float surr_dx2 = surrounding_grads[3].dot(dir1);

        if (surr_dx1 * surr_dx2 > 0 && dzdx * surr_dx1 < 0 &&
            norm.dot(vec3(0,0,1)) < 0.01f &&
            std::abs(surr_dx1 - dzdx) > threshold &&
            std::abs(surr_dx2 - dzdx) > threshold) {
            return true;
        }
    } else return true;
    return false;
}

// Uses cramers rule to compute estimated gradient
vec3 Geometry::computeGradient(vec3 v0, vec3 v1, vec3 v2, bool odd) {
    if (!(v0.isfinite() && v1.isfinite() && v2.isfinite())) return vec3(NAN,NAN,NAN);
    vec3 dx, dy;
    /* First triangle in quad 
       v0
       |\
       | \
     dy|  \
       |   \
       |    \
       |     \
       |______\
       v1 dx v2

       Second triangle in quad
                v0
               /|
              / |
             /  |
            /   | dy
           /    |
          /     |
         /      |
        /_______|
       v1   dx   v2
    */

    if (odd) {
        dx = v2 - v1;
        dy = v0 - v2;
    } else {
        dx = v2 - v0;
        dy = v1 - v0;
    }

    float dz = dx.z;
    float dz2 = dy.z;
    float det =  dx.x * dy.y - dy.x * dx.y;

    if (std::abs(det) < 1e-6f) return vec3(NAN,NAN,NAN);

    float dzdx = (dz * dy.y - dy.x * dz2) / det;
    float dzdy = (dx.x * dz2 - dz * dx.y) / det;

    return vec3(dzdx, dzdy, 0);
}

// Computes gradient of triangle below, left, above, right
std::vector<vec3> Geometry::computeSurroundingGradients(int row, int col) {
    std::vector<vec3> grads;
    if (row > 1) {
        int i0 = (step_ * (row - 1) + col) * 3;
        int i1 = (step_ * (row - 2) + col) * 3;
        int i2 = (step_ * (row - 1) + 1 + col) * 3;
        vec3 v0(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
        vec3 v1(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
        vec3 v2(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
        grads.push_back(computeGradient(v0, v1, v2, false));
    } else {
        grads.push_back(vec3(NAN,NAN,NAN));
    }
    if (col > 0) {
        int i0 = (step_ * row + col) * 3;
        int i1 = (step_ * (row - 1) + col - 1) * 3;
        int i2 = (step_ * (row - 1) + col) * 3;
        vec3 v0(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
        vec3 v1(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
        vec3 v2(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
        grads.push_back(computeGradient(v0, v1, v2, true));
    } else {
        grads.push_back(vec3(NAN,NAN,NAN));
    }
    if (row < step_ - 1) {
        int i0 = (step_ * (row + 1) + 1 + col) * 3;
        int i1 = (step_ * row + col) * 3;
        int i2 = (step_ * row + 1 + col) * 3;
        vec3 v0(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
        vec3 v1(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
        vec3 v2(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
        grads.push_back(computeGradient(v0, v1, v2, true));
    } else {
        grads.push_back(vec3(NAN,NAN,NAN));
    }
    if (col < step_ - 1) {
        int i0 = (step_ * row + 1 + col) * 3;
        int i1 = (step_ * (row - 1) + 1 + col) * 3;
        int i2 = (step_ * row + 2 + col) * 3;
        vec3 v0(vertices_[i0], vertices_[i0 + 1], vertices_[i0 + 2]);
        vec3 v1(vertices_[i1], vertices_[i1 + 1], vertices_[i1 + 2]);
        vec3 v2(vertices_[i2], vertices_[i2 + 1], vertices_[i2 + 2]);
        grads.push_back(computeGradient(v0, v1, v2, false));
    } else {
        grads.push_back(vec3(NAN,NAN,NAN));
    }
    for (int i = 0; i < 4; i++) {
        if (!grads[i].isfinite()) {
            if (i < 2) {
                grads[i] = grads[i + 2];
            } else {
                grads[i] = grads[i - 2];
            }
        }
    }
    return grads;
}

void Geometry::pushVertex(vec3 v0, vec3 v1, vec3 v2, std::vector<float>& verts) {
    verts.insert(verts.end(), {v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z});
};

// Accumlate normals for triangles that share vertices
void Geometry::pushNormal(vec3 v0, vec3 v1, vec3 v2, std::unordered_map<vec3, vec3, vec3::Vec3Hash>& normal_map) {
    vec3 v1_to_v0 = v0 - v1;
    vec3 v1_to_v2 = v2 - v1;
    vec3 normal = v1_to_v2.cross(v1_to_v0);
    normal_map[v0] += normal;
    normal_map[v1] += normal;
    normal_map[v2] += normal;
}
