#ifndef DATABASE_DATABASE_H
#define DATABASE_DATABASE_H

#include "PreRequistion.h"
#include "DBQLParser.h"

// Definicje typów danych
enum class DataType {
    INT, FLOAT, STRING, BOOL
};

DataType getTypeFromString(const std::string &typeString);


// Struktura reprezentująca kolumnę
struct Table {
    struct Column {
        std::string name;
        DataType type;
        int index;


        Column(const std::string &columnName, DataType columnType, int columnIndex)
                : name(columnName), type(columnType), index(columnIndex) {}

        bool isValidType(const std::string &value) const;
    };

    std::string name;
    std::vector<std::vector<std::string>> data; // Change from std::map to std::vector
    std::vector<Column> columns;

    int getConditionColumnIndex(const std::string &conditionColumn);
    bool isValidColumnType(const Column &column);
};


class Database {
public:
    Database();


    // Metody DDL
    auto createTable(const TableDefinition& tableDef) -> void;

    void dropTable(const std::string &tableName);

    auto addColumn(const std::string &tableName, const ColumnDefinition &colDef) -> void ;

    void addNewColumn(const std::string& tableName, const std::string& columnName, DataType columnType);

    auto removeColumn(const std::string &tableName, const std::string &columnName) -> void;

    // Metody DML
    auto insertData(const std::string &tableName, const std::map<std::string, std::string> &rowData) -> bool;

    auto updateData(const std::string &tableName, const std::vector<std::pair<std::string, std::string>> &updateValues,
                              const std::string &conditionColumn, const std::string &conditionValue) -> bool;

    auto tableExists(const std::string& tableName) const -> bool;
    void deleteData(const std::string &tableName, const std::string &conditionColumn, const std::string &conditionValue);

    // Metody DQL
    auto selectData(const std::string &tableName, const std::vector<std::string> &columns, const std::string &condition) -> void ;




    // Zapis/Odczyt
    auto saveToFile(const std::string &fileName) -> void;

    auto loadDataFromFile(const std::string &fileName) -> void;

    auto executeQuery(const std::string &query) -> void;


    std::map<std::string, Table> tables;
    // Mapa nazwa tabeli -> tabela

};


#endif //DATABASE_DATABASE_H
