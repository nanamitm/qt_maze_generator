#ifndef GENERATORCATALOG_H
#define GENERATORCATALOG_H

#include <QString>

#include <functional>
#include <memory>
#include <random>
#include <vector>

class MazeGenerator;

// One entry per maze generation algorithm. Adding an algorithm means writing
// its class and appending a row to the catalog: the algorithm combo box, the
// status messages and the self-test all read from here, so nothing else needs
// to know the list.
struct GeneratorInfo {
    QString id;          // stable identifier, safe to persist
    QString displayName; // shown in the UI
    std::function<std::unique_ptr<MazeGenerator>(std::mt19937 &)> make;
};

const std::vector<GeneratorInfo>& generatorCatalog();

// Returns nullptr when the id is unknown.
const GeneratorInfo* findGenerator(const QString& id);

QString defaultGeneratorId();

#endif // GENERATORCATALOG_H
