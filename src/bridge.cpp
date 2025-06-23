#include "bridge.hpp"
#include "geometry.hpp"

bool Bridge::updateEvaluator(const QString &latex, const QString &id, const QVariantMap &vars, QVariant step_q, QVariant range_q, QVariant clip_z) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    int step = step_q.toInt();
    int range = range_q.toInt();
    bool clip = clip_z.toBool();

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
            generateMeshASync(id, range, step, clip);
            return true;
        } catch (const std::exception& e) {
            qDebug() << "Error updating AST:" << e.what();
            return false;
        }
    } else {
        return createEvaluator(latex, id, vars, step, range, clip);
    }
}

bool Bridge::createEvaluator(const QString &latex, const QString &id, const QVariantMap &vars, QVariant step_q, QVariant range_q, QVariant clip_z) {
    // Normalize the ID to ensure consistent hashing
    QString norm = id.trimmed().normalized(QString::NormalizationForm_C);
    int step = step_q.toInt();
    int range = range_q.toInt();
    bool clip = clip_z.toBool();

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
        generateMeshASync(id, range, step, clip);
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
        return true;
    }
    return false;
}

void Bridge::updateMesh(int range, int step, bool clip_z) {
    for (std::pair<QString, Evaluator*> pair : evaluators_) {
        QString id = pair.first;
        Evaluator* evaluator = pair.second;

        // Update the geometry with the new range and step
        generateMeshASync(id, range, step, clip_z);
        qDebug() << "Step and range updated for ID:" << id;
    }
}

void Bridge::generateMeshASync(const QString& id, int range, int step, bool clip_z) {
    if (!evaluators_.count(id)) return;
    
    Evaluator* evaluator = evaluators_[id];
    long long job_id = ++latest_id_;

    auto future = QtConcurrent::run([this, evaluator, step, range, clip_z]() -> std::pair<std::vector<float>, std::vector<float>> {
        try {
            Geometry geometry(evaluator, step, range, clip_z);
            return std::make_pair(geometry.vertices_, geometry.normals_);
        } catch (const std::exception& e) {
            qDebug() << "Error generating mesh: " << e.what();
            return std::make_pair(std::vector<float>(), std::vector<float>());
        }
    });

    auto watcher = new QFutureWatcher<std::pair<std::vector<float>, std::vector<float>>>(this);
    connect(watcher, &QFutureWatcher<std::pair<std::vector<float>, std::vector<float>>>::finished, 
            [this, job_id, watcher, id]() {
        auto result = watcher->result();
        if (result.first.empty() && result.second.empty()) return;

        QByteArray raw_vertices(reinterpret_cast<const char*>(result.first.data()), result.first.size() * sizeof(float));
        QString vertices_base64 = QString::fromLatin1(raw_vertices.toBase64());

        QByteArray raw_normals(reinterpret_cast<const char*>(result.second.data()), result.second.size() * sizeof(float));
        QString normals_base64 = QString::fromLatin1(raw_normals.toBase64());
        
        qDebug() << "Mesh updated for ID:" << id;
        // Ensure older threads that finish later than newer ones don't overwrite new data
        if (job_id >= latest_completed_id_) {
            latest_completed_id_ = job_id;
            emit meshUpdated(id, vertices_base64, normals_base64);
        }
        
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void Bridge::print(const QString& str) {
    qDebug() << str;
}