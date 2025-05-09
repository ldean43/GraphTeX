#include "bridge.hpp"
#include "geometry.hpp"

QVariantList Bridge::getVertices(const QString &id) {
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    QVariantList list;
    for (float f : vertices_[norm]) {
        list.append(f);
    }
    return list;
}

QVariantList Bridge::getNormals(const QString &id) {
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    QVariantList list;
    for (float f : normals_[norm]) {
        list.append(f);
    }
    return list;
}

QVariantList Bridge::getIndices(const QString &id) {
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    QVariantList list;
    for (uint16_t i : indices_[norm]) {
        list.append(i);
    }
    return list;
}

bool Bridge::updateEvaluator(const QString &latex, const QString &id, const QVariantMap &vars) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);

    if (latex.isEmpty()) {
        qDebug() << "Error: Empty LaTeX string";
        return false;
    }

    if (evaluators_.count(norm)) {
        try {
            Lexer lexer(latex.toStdString());
            Parser parser(lexer.lex());
            Expr* ast = parser.parse()->copy();
            
            delete evaluators_[norm]->ast_;
            evaluators_[norm]->ast_ = ast;
            evaluators_[norm]->vars_.clear();

            // Convert passed javascript object for variables into unordered_map<string, float>
            for (auto it = vars.begin(); it != vars.end(); ++it) {
                if (it.value().canConvert<float>()) {
                    evaluators_[norm]->vars_[it.key().toStdString()] = it.value().toFloat();
                }
            }

            // Generate the mesh
            Geometry geometry(evaluators_[norm]);
            qDebug() << geometry.vertices_.size();
            vertices_[norm] = geometry.vertices_;
            indices_[norm] = geometry.indices_;
            normals_[norm] = geometry.normals_;
            qDebug() << "Mesh updated for ID:" << norm;
            return true;
        } catch (const std::exception& e) {
            qDebug() << "Error updating AST:" << e.what();
            return false;
        }
    } else {
        return createEvaluator(latex, id, vars);
    }
}

bool Bridge::createEvaluator(const QString &latex, const QString &id, const QVariantMap &vars) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);

    try {
        // Create a new evaluator
        Lexer lexer(latex.toStdString());
        Parser parser(lexer.lex());
        Expr* ast = parser.parse()->copy();
        evaluators_[norm] = new Evaluator(ast, {});

        // Convert passed javascript object for variables into unordered_map<string, float>
        for (auto it = vars.begin(); it != vars.end(); ++it) {
            if (it.value().canConvert<float>()) {
                evaluators_[norm]->vars_[it.key().toStdString()] = it.value().toFloat();
            }
        }

        // Generate the mesh
        Geometry geometry(evaluators_[norm]);
        vertices_[norm] = geometry.vertices_;
        normals_[norm] = geometry.normals_;
        indices_[norm] = geometry.indices_;
        qDebug() << "Mesh updated for ID:" << norm;
        return true;
    } catch (const std::exception& e) {
        // Handle lexing and parsing errors
        qDebug() << "Error creating AST:" << e.what();
        return false;
    }
}

bool Bridge::deleteEvaluator(const QString &id) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    if (evaluators_.count(norm)) {
        delete evaluators_[norm];
        evaluators_.erase(norm);
        vertices_.erase(norm);
        indices_.erase(norm);
        return true;
    }
    return false;
}