#include "bridge.hpp"
#include "geometry.hpp"

QByteArray Bridge::getMesh(const QString &id) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    return QByteArray(reinterpret_cast<const char*>(meshes_[norm].data()), meshes_[norm].size() * sizeof(float));
}

QByteArray Bridge::getIndices(const QString &id) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    return QByteArray(reinterpret_cast<const char*>(indices_[norm].data()), indices_[norm].size() * sizeof(uint16_t));
}

void Bridge::updateEvaluator(const QString &latex, const QString &id, const QVariantMap &vars) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);

    if (latex.isEmpty()) {
        qDebug() << "Error: Empty LaTeX string";
        return;
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
            geometry.generateVertices();
            geometry.generateIndices();
            meshes_[norm].clear();
            indices_[norm].clear();
            qDebug() << geometry.vertices_.size();
            for (auto it = geometry.vertices_.begin(); it < geometry.vertices_.end(); it += 3) {
                qDebug() << *it << *(it + 1) << *(it + 2);
            }

            meshes_[norm] = geometry.vertices_;
            indices_[norm] = geometry.indices_;
            qDebug() << "Mesh updated for ID:" << norm;
        } catch (const std::exception& e) {
            qDebug() << "Error updating AST:" << e.what();
        }
    } else {
        createEvaluator(latex, id, vars);
    }
    return;
}

void Bridge::createEvaluator(const QString &latex, const QString &id, const QVariantMap &vars) {
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
        geometry.generateVertices();
        geometry.generateIndices();
        meshes_[norm] = geometry.vertices_;
        indices_[norm] = geometry.indices_;
        qDebug() << "Mesh updated for ID:" << norm;
    } catch (const std::exception& e) {
        // Handle lexing and parsing errors
        qDebug() << "Error creating AST:" << e.what();
    }
    return;
}

void Bridge::deleteEvaluator(const QString &id) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    if (evaluators_.count(norm)) {
        delete evaluators_[norm];
        evaluators_.erase(norm);
        meshes_.erase(norm);
        indices_.erase(norm);
    }
}