//
// Created by rozsh on 10/01/2024.
//

#ifndef DATABASE_DATABASE_H
#define DATABASE_DATABASE_H
#include "PreRequistion.h"

// Definicje typów danych
enum class DataType { INT, FLOAT, STRING };

// Struktura reprezentująca kolumnę
struct Column {
    std::string name;
    DataType type;
    bool isValidType(const std::string& value) const;


};

// Struktura reprezentująca tabelę
struct Table {
    std::string name;
    std::map<std::string, Column> columns;
    std::map<std::string, std::vector<std::string>> data; // Mapa kolumna -> lista wartości
};

class Database {
public:
    Database();
    virtual ~Database();

    // Metody DDL
    void createTable(const std::string& tableName);
    void dropTable(const std::string& tableName);
    void addColumn(const std::string& tableName, const Column& column);
    void removeColumn(const std::string& tableName, const std::string& columnName);

    // Metody DML
    void insertData(const std::string& tableName, const std::map<std::string, std::string>& rowData);
    void updateData(const std::string& tableName, const std::map<std::string, std::string>& updateValues, const std::string& conditionColumn, const std::string& conditionValue);
    void deleteData(const std::string& tableName, const std::string& conditionColumn, const std::string& conditionValue);

    // Metody DQL
    void selectData(const std::string& tableName, const std::vector<std::string>& columns, const std::string& condition);

    // Zapis/Odczyt
    void saveToFile(const std::string& fileName);
    void loadDataFromFile(const std::string& fileName);
    void executeQuery(const std::string& query);
private:
    std::map<std::string, Table> tables; // Mapa nazwa tabeli -> tabela
};


#endif //DATABASE_DATABASE_H
