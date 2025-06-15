#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <cstdint>
#include <unordered_map>
#include "InTeX/ast.hpp"
#include "InTeX/lexer.hpp"
#include "InTeX/parser.hpp"
#include "InTeX/evaluator.hpp"

class Bridge : public QObject
{
    Q_OBJECT
public:
    std::unordered_map<QString, Evaluator*> evaluators_;
    std::unordered_map<QString, std::vector<float>> vertices_;
    std::unordered_map<QString, std::vector<float>> normals_;

    explicit Bridge(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    QString getVertices(const QString &id);
    QString getNormals(const QString &id);
    bool updateEvaluator(const QString &latex, const QString &id, const QVariantMap &vars, QVariant step_q, QVariant range_q, QVariant clip_z);
    bool createEvaluator(const QString &latex, const QString &id, const QVariantMap &vars, QVariant step_q, QVariant range_q, QVariant clip_z);
    bool deleteEvaluator(const QString &id);
    void updateMesh(int range, int step, bool clip_z);
};