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
    std::unordered_map<QString, std::vector<float>> meshes_;
    std::unordered_map<QString, std::vector<uint16_t>> indices_;

    explicit Bridge(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    QByteArray getMesh(const QString &id);
    QByteArray getIndices(const QString &id);
    void updateEvaluator(const QString &latex, const QString &id, const QVariantMap &vars);
    void createEvaluator(const QString &latex, const QString &id, const QVariantMap &vars);
    void deleteEvaluator(const QString &id);
};