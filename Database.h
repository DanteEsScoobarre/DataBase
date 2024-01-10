#ifndef DATABASE_DATABASE_H
#define DATABASE_DATABASE_H

#include "PreRequistion.h"

// Definicje typów danych
enum class DataType {
    INT, FLOAT, STRING
};

DataType getTypeFromString(const std::string &typeString);


// Struktura reprezentująca kolumnę
struct Column {
    std::string name;
    DataType type;
    Column() : index(-1){}

    Column(const std::string &columnName, DataType columnType, int columnIndex)
            : name(columnName), type(columnType), index(columnIndex) {}

    bool isValidType(const std::string &value) const;

    int index;


};

// Struktura reprezentująca tabelę
struct Table {
    std::string name;
    std::map<std::string, std::vector<std::string>> data;
    std::map<std::string, Column> columns;


    int getConditionColumnIndex(const std::string &conditionColumn); // Mapa kolumna -> lista wartości
    bool isValidColumnType(const Column &column);
};

class Database {
public:
    Database();


    // Metody DDL
    void createTable(const std::string &tableName);

    void dropTable(const std::string &tableName);

    void addColumn(const std::string &tableName, const Column &column);

    void addNewColumn(const std::string& tableName, const std::string& columnName, DataType columnType);

    void removeColumn(const std::string &tableName, const std::string &columnName);

    // Metody DML
    void insertData(const std::string &tableName, const std::map<std::string, std::string> &rowData);

    void updateData(const std::string &tableName, const std::map<std::string, std::string> &updateValues,
                    const std::string &conditionColumn, const std::string &conditionValue);

    void
    deleteData(const std::string &tableName, const std::string &conditionColumn, const std::string &conditionValue);

    // Metody DQL
    void
    selectData(const std::string &tableName, const std::vector<std::string> &columns, const std::string &condition);




    // Zapis/Odczyt
    //void saveToFile(const std::string &fileName);

    // void loadDataFromFile(const std::string &fileName);

    void executeQuery(const std::string &query);


private:
    std::map<std::string, Table> tables; // Mapa nazwa tabeli -> tabela
};


#endif //DATABASE_DATABASE_H
