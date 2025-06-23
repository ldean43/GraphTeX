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
#include <cmath>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>

class Bridge : public QObject
{
    Q_OBJECT
public:
    explicit Bridge(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    bool updateEvaluator(const QString &latex, const QString &id, const QVariantMap &vars, QVariant step_q, QVariant range_q, QVariant clip_z);
    bool createEvaluator(const QString &latex, const QString &id, const QVariantMap &vars, QVariant step_q, QVariant range_q, QVariant clip_z);
    bool deleteEvaluator(const QString &id);
    void updateMesh(int range, int step, bool clip_z);
    void print(const QString &str);

signals:
    void meshUpdated(const QString &id, QString vertices_base64, QString normals_base64);

private:
    std::unordered_map<QString, Evaluator*> evaluators_;
    void generateMeshASync(const QString& id, int step, int range, bool clip_z);
    std::atomic<long long> latest_id_ = 0;
    std::atomic<long long> latest_completed_id_ = 0;
};