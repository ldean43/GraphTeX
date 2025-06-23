#include "InTeX/ast.hpp"
#include "InTeX/lexer.hpp"
#include "InTeX/parser.hpp"
#include "InTeX/evaluator.hpp"
#include "vec3.hpp"
#include <QDebug>
#include <chrono>
#include <iostream>
#include <thread>
#include <cstdint>
#include <algorithm>
#include <mutex>
#include <vector>

class Geometry {
private:
    Evaluator* evaluator_;
    int step_;
    int range_;
    double step_size_;
    std::mutex mutex_;

    void generateVertices(int minrow, int maxrow);
    void clipTriangles(int minrow, int maxrow, bool clip);
    bool crossDiscontinuity(vec3 v0, vec3 v1, vec3 v2, std::vector<vec3> surrounding_grads, bool odd);
    vec3 computeGradient(vec3 v0, vec3 v1, vec3 v2, bool odd);
    std::vector<vec3> computeSurroundingGradients(int row, int col);
    void pushVertex(vec3 v0, vec3 v1, vec3 v2, std::vector<float>& verts);
    void pushNormal(vec3 v0, vec3 v1, vec3 v2, std::unordered_map<vec3, vec3, vec3::Vec3Hash>& normal_map);
public:
    std::vector<float> tempVertices_;
    std::vector<float> vertices_;
    std::vector<float> normals_;
    explicit Geometry(Evaluator* evaluator, int step, int range, bool clip);
    ~Geometry() {}
};