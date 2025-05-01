#include "geometry.hpp"

void Geometry::generateVertices() {
    // truncate small decimals
    const float epsilon = 1e-6;

    if((evaluator_->vars_.find("x") == evaluator_->vars_.end())
        && (evaluator_->vars_.find("y") == evaluator_->vars_.end())) {
        graph_ = false;
        result_ = evaluator_->evaluate();
        return;
    }

    for (float y = -range_; y < range_; y += range_ * 0.1) {
        for (float x = -range_; x < range_; x += range_ * 0.1) {
            x = std::abs(x) < epsilon ? 0.0 : x;
            y = std::abs(y) < epsilon ? 0.0 : y;
            evaluator_->vars_["x"] = x;
            evaluator_->vars_["y"] = y;
            vertices_.push_back(x);
            vertices_.push_back(y);
            vertices_.push_back(evaluator_->evaluate());
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
    for (int i = 0; i < step_; i++) {
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
                    if (i + 1 <= step_) {
                        indices_.push_back((i+1)*step_ + j);
                        indices_.push_back(i*step_ + j - 1);
                    }
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
                if (i + 1 < step_) {
                    indices_.push_back((i+1)*step_ + j);
                    indices_.push_back(i*step_ + j);
                }
            }
        }
    }
}