#include "InTeX/ast.hpp"
#include "InTeX/lexer.hpp"
#include "InTeX/parser.hpp"
#include "InTeX/evaluator.hpp"
#include <cstdint>

class Geometry {
private:
    Evaluator* evaluator_;
    bool graph_;
    float result_;
    int range_;
    int step_;
public:
    std::vector<float> vertices_; // All Vertices
    std::vector<uint16_t> indices_; // Triangle Strip Indices
    
    explicit Geometry(Evaluator* evaluator) : evaluator_(evaluator) {
        range_ = 1;
        step_ = (int)((2*(float)range_)/((float)range_ * 0.1));
    }
    explicit Geometry(Evaluator* evaluator, float range) : evaluator_(evaluator) {
        range_ = range;
        step_ = (2*range)/(range_ * 0.01);
    }
    ~Geometry() {}
    void generateVertices();
    /*
        Generates indices for triangle strip, alternating from left to right 
        and right to left each row
    */
    void generateIndices();
};