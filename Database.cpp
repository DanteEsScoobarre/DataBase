#include "Database.h"
#include "DBQLParser.h"

Database::Database() {

}

void Database::createTable(const std::string &tableName) {
    if (tables.find(tableName) != tables.end()) {
        std::cerr << "Table " << tableName << " already exists." << std::endl;
        return;
    }
    tables[tableName] = Table{tableName};
}


void Database::dropTable(const std::string &tableName) {
    if (tables.find(tableName) == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }
    tables.erase(tableName);

}

void Database::addColumn(const std::string &tableName, const Column &column) {
    auto table = tables.find(tableName);
    if (table == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }
    if (table->second.columns.find(column.name) != table->second.columns.end()) {
        std::cerr << "Column " << column.name << " already exists in table " << tableName << "." << std::endl;
        return;
    }
    table->second.columns[column.name] = column;

}

void Database::removeColumn(const std::string &tableName, const std::string &columnName) {
    auto table = tables.find(tableName);
    if (table == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }
    if (table->second.columns.find(columnName) == table->second.columns.end()) {
        std::cerr << "Column " << columnName << " does not exist in table " << tableName << "." << std::endl;
        return;
    }
    table->second.columns.erase(columnName);

}

void Database::insertData(const std::string &tableName, const std::map<std::string, std::string> &rowData) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Sprawdzanie, czy typy danych są zgodne
    for (const auto& col : rowData) {
        auto colIt = tableIt->second.columns.find(col.first);
        if (colIt == tableIt->second.columns.end()) {
            std::cerr << "Column " << col.first << " does not exist in table " << tableName << "." << std::endl;
            return;
        }
        if (!colIt->second.isValidType(col.second)) {
            std::cerr << "Invalid type for column " << col.first << "." << std::endl;
            return;
        }
    }

    // Dodawanie danych
    for (const auto& col : rowData) {
        tableIt->second.data[col.first].push_back(col.second);
    }
}

void Database::updateData(const std::string& tableName, const std::map<std::string, std::string>& updateValues, const std::string& conditionColumn, const std::string& conditionValue) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Sprawdzanie, czy kolumna warunku istnieje
    auto colIt = tableIt->second.columns.find(conditionColumn);
    if (colIt == tableIt->second.columns.end()) {
        std::cerr << "Condition column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    // Przechodzenie przez wszystkie wiersze i aktualizacja danych
    for (auto& row : tableIt->second.data) {
        auto& values = row.second;
        if (values.find(conditionColumn) != values.end() && values[conditionColumn] == conditionValue) {
            for (const auto& colVal : updateValues) {
                if (tableIt->second.columns.find(colVal.first) != tableIt->second.columns.end()) {
                    values[colVal.first] = colVal.second;
                }
            }
        }
    }
}



void Database::deleteData(const std::string& tableName, const std::string& conditionColumn, const std::string& conditionValue) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Sprawdzanie, czy kolumna warunku istnieje
    if (tableIt->second.columns.find(conditionColumn) == tableIt->second.columns.end()) {
        std::cerr << "Condition column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    // Usuwanie wierszy spełniających warunek
    for (auto it = tableIt->second.data.begin(); it != tableIt->second.data.end();) {
        if (it->second[conditionColumn] == conditionValue) {
            it = tableIt->second.data.erase(it);
        } else {
            ++it;
        }
    }
}

void Database::selectData(const std::string &tableName, const std::vector<std::string> &columns,
                          const std::string &condition) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Parsowanie warunku (na razie bardzo proste)
    std::string conditionColumn;
    std::string conditionValue;
    if (!condition.empty()) {
        size_t equalPos = condition.find('=');
        if (equalPos != std::string::npos) {
            conditionColumn = condition.substr(0, equalPos);
            conditionValue = condition.substr(equalPos + 1);
        }
    }

    // Wyświetlanie danych
    for (const auto& row : tableIt->second.data[conditionColumn]) {
        bool match = condition.empty() || (row == conditionValue);
        if (match) {
            for (const auto& col : columns) {
                std::cout << row << " ";
            }
            std::cout << std::endl;
        }
    }
}


void Database::saveToFile(const std::string &fileName) {

}

void Database::loadDataFromFile(const std::string &fileName) {

}

bool Column::isValidType(const std::string &value) const {
    switch (type) {
        case DataType::INT: {
            char *end;
            std::strtol(value.c_str(), &end, 10);
            return *end == '\0';
        }
        case DataType::FLOAT: {
            char *end;
            std::strtof(value.c_str(), &end);
            return *end == '\0';
        }
        case DataType::STRING:
            return true;
        default:
            return false;
    }
}

void Database::executeQuery(const std::string& query){
    DBQLParser dbqlParser(query);


    // Otrzymujemy elementy zapytania
    auto tableName = dbqlParser.getTableName();
    auto columns = dbqlParser.getColumns();
    auto condition = dbqlParser.getCondition();

    // Wykonanie zapytania
    selectData(tableName, columns, condition);
}