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

    if (!table->second.isValidColumnType(column)) {
        std::cerr << "Invalid type for column " << column.name << " in table " << tableName << "." << std::endl;
        return;
    }

    // Utwórz nowy obiekt Column używając konstruktora
    Column newColumn(column.name, column.type, static_cast<int>(table->second.columns.size()));
    table->second.columns.emplace(newColumn.name, newColumn);
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
        auto collIt = tableIt->second.columns.find(col.first);
        if (collIt == tableIt->second.columns.end()) {
            std::cerr << "Column " << col.first << " does not exist in table " << tableName << "." << std::endl;
            return;
        }
        if (!collIt->second.isValidType(col.second)) {
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

    // Sprawdzanie, czy kolumna warunku istnieje i uzyskanie jej indeksu
    auto updatecolIt = tableIt->second.columns.find(conditionColumn);
    int conditionColumnIndex = updatecolIt->second.index;

    // Przechodzenie przez wszystkie wiersze i aktualizacja danych
    for (auto& row : tableIt->second.data) {
        if (row.second.size() > conditionColumnIndex && row.second[conditionColumnIndex] == conditionValue) {
            for (const auto& colVal : updateValues) {
                auto updateColIt = tableIt->second.columns.find(colVal.first); // Fix: Rename conflicting variable 'colIt' to 'updateColIt'
                if (updateColIt != tableIt->second.columns.end()) {
                    int updateColumnIndex = updateColIt->second.index; // Fix: Access the index member of the Column object
                    if (row.second.size() > updateColumnIndex) {
                        row.second[updateColumnIndex] = colVal.second;
                    }
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

    auto conditionColumnIt = tableIt->second.columns.find(conditionColumn);
    if (conditionColumnIt == tableIt->second.columns.end()) {
        std::cerr << "Column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    int conditionColumnIndex = conditionColumnIt->second.index;

    auto& data = tableIt->second.data;
    std::map<std::string, std::vector<std::string>> newData;
    for (const auto& row : data) {
        if (row.second[conditionColumnIndex] != conditionValue) {
            newData.insert(row);
        }
    }
    data = std::move(newData);
}


void Database::selectData(const std::string& tableName, const std::vector<std::string>& columns,
                          const std::string& condition) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Parsowanie warunku (na razie bardzo proste)
    std::vector<Condition> conditions;
    DBQLParser::parseConditions(condition, conditions);

    // Wyświetlanie danych
    for (const auto& row : tableIt->second.data) {
        bool meetConditions = true;

        // Sprawdzanie, czy wiersz spełnia warunki
        for (const auto& cond : conditions) {
            size_t columnIndex = std::find(columns.begin(), columns.end(), cond.column) - columns.begin();

            if (columnIndex < columns.size()) {
                if (cond.op == "==" && row.second.at(columnIndex) != cond.value) { // Fix: Access the element using the at() function
                    meetConditions = false;
                    break;
                }
                // Dodaj obsługę innych operatorów porównania, np. "<", ">", itp., jeśli jest to wymagane
            } else {
                std::cerr << "Column " << cond.column << " not found in specified columns." << std::endl;
                meetConditions = false;
                break;
            }
        }

        if (meetConditions) {
            for (size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                std::cout << row.second.at(columnIndex) << " "; // Fix: Access the element using the at() function
            }
            std::cout << std::endl;
        }
    }
}


void Database::saveToFile(const std::string &fileName) {
    std::ofstream outputFile(fileName);
    if (!outputFile) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return;
    }

    for (const auto& table : tables) {
        outputFile << "Table: " << table.first << std::endl;

        // Write column names
        for (const auto& column : table.second.columns) {
            outputFile << column.first << ",";
        }
        outputFile << std::endl;

        // Write data rows
        for (const auto& row : table.second.data) {
            for (const auto& value : row.second) {
                outputFile << value << ",";
            }
            outputFile << std::endl;
        }

        outputFile << std::endl;
    }

    outputFile.close();
}

/*void Database::loadDataFromFile(const std::string &fileName) {
    std::ifstream inputFile(fileName);
    if (!inputFile) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return;
    }

    std::string line;
    std::string tableName;
    std::map<std::string, Column> columns;
    std::vector<std::vector<std::string>> data;

    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            if (!tableName.empty()) {
                tables.emplace(tableName, Table{tableName, columns, std::map<std::string, std::vector<std::string>>{data.begin(), data.end()}});
                tableName.clear();
                columns.clear();
                data.clear();
            }
        } else if (line.substr(0, 6) == "Table:") {
            tableName = line.substr(7);
        } else if (tableName.empty()) {
            std::istringstream iss(line);
            std::string columnName;
            while (std::getline(iss, columnName, ',')) {
                columns[columnName] = {columnName};
            }
        } else {
            std::istringstream iss(line);
            std::string value;
            std::vector<std::string> rowData;
            while (std::getline(iss, value, ',')) {
                rowData.push_back(value);
            }
            data.push_back(rowData);
        }
    }

    if (!tableName.empty()) {
        std::map<std::string, std::vector<std::string>> convertedData(data.begin(), data.end());
        tables.emplace(tableName, Table{tableName, columns, convertedData});
    }

    inputFile.close();
}
 */
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

int Table::getConditionColumnIndex(const std::string &conditionColumn){
    auto colIt = columns.find(conditionColumn);
    if (colIt == columns.end()) {
        std::cerr << "Condition column " << conditionColumn << " does not exist in table " << name << "." << std::endl;
        return -1;
    }
    else {
        return colIt->second.index;
    }
}

bool Table::isValidColumnType(const Column &column) {
    for (const auto& existingColumn : columns) {
        if (existingColumn.second.type != column.type) {
            return false;
        }
    }
    return true;
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

void Database::addNewColumn(const std::string& tableName, const std::string& columnName, DataType columnType) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Sprawdzenie, czy kolumna o podanej nazwie już istnieje
    if (tableIt->second.columns.find(columnName) != tableIt->second.columns.end()) {
        std::cerr << "Column " << columnName << " already exists in table " << tableName << "." << std::endl;
        return;
    }

    // Dodanie nowej kolumny do istniejącej tabeli
    int columnIndex = static_cast<int>(tableIt->second.columns.size());
    tableIt->second.columns[columnName] = Column{columnName, columnType, columnIndex};


    // Zainicjowanie pustych wartości dla nowej kolumny we wszystkich wierszach
    for (auto& row : tableIt->second.data) {
        row.second.push_back("");
    }
}


