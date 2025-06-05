#include "InTeX/ast.hpp"
#include "InTeX/lexer.hpp"
#include "InTeX/parser.hpp"
#include "InTeX/evaluator.hpp"
#include "vec3.hpp"
#include <QDebug>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>

class Geometry {
private:
    Evaluator* evaluator_;
    int step_;
    int range_;
    double step_size_;

    bool crossDiscontinuity(vec3 v0, vec3 v1, vec3 v2);
    void pushVertex(vec3 v0, vec3 v1, vec3 v2, std::vector<float>& verts);
    void generateVertices();
    void clipTriangles();
    void generateNormals();
public:
    std::vector<float> vertices_; // All Vertices
    std::vector<uint32_t> indices_; // Triangle Strip Indices
    std::vector<float> normals_; // Normals
    
    explicit Geometry(Evaluator* evaluator) : evaluator_(evaluator) {
        auto start = std::chrono::high_resolution_clock::now();
        range_ = 10;
        step_ = 250;
        step_size_ = 2.0f * range_ / (step_ - 1);
        vertices_.resize(3 * step_ * step_);
        auto end = std::chrono::high_resolution_clock::now();
        generateVertices();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "generateVertices finished in: " << elapsed.count() << " seconds.\n";
        generateNormals();
    }
    explicit Geometry(Evaluator* evaluator, float range) : evaluator_(evaluator) {
        range_ = range;
        step_ = (2*range)/(range_ * 0.01);
        generateNormals();
    }
    ~Geometry() {}
};