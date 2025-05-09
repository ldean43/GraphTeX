#include "InTeX/ast.hpp"
#include "InTeX/lexer.hpp"
#include "InTeX/parser.hpp"
#include "InTeX/evaluator.hpp"
#include "vec3.hpp"
#include <cstdint>

class Geometry {
private:
    Evaluator* evaluator_;
    bool graph_;
    float result_;
    int range_;
    int step_;

    void generateVertices();
    void generateIndices();
    void generateNormals();
public:
    std::vector<float> vertices_; // All Vertices
    std::vector<uint16_t> indices_; // Triangle Strip Indices
    std::vector<float> normals_; // Normals
    
    explicit Geometry(Evaluator* evaluator) : evaluator_(evaluator) {
        range_ = 10;
        step_ = (int)((2*(float)range_)/((float)range_ * .01));
        this->generateVertices();
        this->generateIndices();
        this->generateNormals();
    }
    explicit Geometry(Evaluator* evaluator, float range) : evaluator_(evaluator) {
        range_ = range;
        step_ = (2*range)/(range_ * 0.01);
        this->generateVertices();
        this->generateIndices();
        this->generateNormals();
    }
    ~Geometry() {}
};