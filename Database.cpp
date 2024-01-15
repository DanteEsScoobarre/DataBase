#include "Database.h"
#include "DBQLParser.h"


Database::Database() {

}


auto Database::addColumn(const std::string &tableName, const ColumnDefinition &colDef) -> void {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &table = tableIt->second;

    if (std::ranges::find_if(table.columns.begin(), table.columns.end(),
                             [&colDef](const Table::Column &existingColumn) {
                                 return existingColumn.name == colDef.name;
                             }) !=
        table.columns.end()) {
        std::cerr << "Column " << colDef.name << " already exists in table " << tableName << "." << std::endl;
        return;
    }

    // Assuming Table class has a method to validate column type
    if (!table.isValidColumnType(colDef.dataType)) {
        std::cerr << "Invalid type for column " << colDef.name << " in table " << tableName << "." << std::endl;
        return;
    }

    // Create new Column object using the constructor
    Table::Column newColumn(colDef.name, colDef.dataType, static_cast<int>(table.columns.size()));
    table.columns.push_back(newColumn);
}


auto Database::createTable(const TableDefinition &tableDef) -> void {
    if (tables.find(tableDef.tableName) != tables.end()) {
        std::cerr << "Table " << tableDef.tableName << " already exists." << std::endl;
        return;
    }

    Table newTable;
    newTable.name = tableDef.tableName;
    int columnIndex = 0;
    for (const auto &colDef: tableDef.columns) {
        newTable.columns.push_back(Table::Column(colDef.name, colDef.dataType, columnIndex++));
    }

    tables[tableDef.tableName] = std::move(newTable);
}



auto Database::dropTable(const std::string &tableName) -> void {
    if (tables.find(tableName) == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }
    tables.erase(tableName);

}

auto Database::removeColumn(const std::string &tableName, const std::string &columnName) -> void {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &columns = tableIt->second.columns;
    auto columnIt = std::ranges::find_if(columns.begin(), columns.end(),
                                         [&columnName](const Table::Column &column) {
                                             return column.name == columnName;
                                         });

    if (columnIt == columns.end()) {
        std::cerr << "Column " << columnName << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    // Erase the column from the columns vector
    columns.erase(columnIt);

    // Erase the corresponding values in each row of the data vector
    for (auto &row: tableIt->second.data) {
        row.erase(row.begin() + columnIt->index);
    }

    // Update the indices of remaining columns
    for (size_t i = columnIt->index; i < columns.size(); ++i) {
        columns[i].index = static_cast<int>(i);
    }
}

auto Database::insertData(const std::string &tableName, const std::map<std::string, std::string> &rowData) -> bool {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return false;
    }

    std::vector<std::string> newRow(tableIt->second.columns.size(), "");

    for (const auto &col: rowData) {
        auto colIt = std::ranges::find_if(tableIt->second.columns.begin(), tableIt->second.columns.end(),
                                          [&col](const Table::Column &column) { return column.name == col.first; });

        if (colIt != tableIt->second.columns.end()) {
            newRow[colIt->index] = col.second;
        } else {
            std::cerr << "Column " << col.first << " does not exist in table " << tableName << "." << std::endl;
            return false;
        }
    }

    tableIt->second.data.push_back(newRow);
    return true;
}

auto
Database::updateData(const std::string &tableName, const std::vector<std::pair<std::string, std::string>> &updateValues,
                     const std::string &conditionColumn, const std::string &conditionValue) -> bool {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return false;
    }


    auto &columns = tableIt->second.columns;
    auto conditionColumnIt = std::ranges::find_if(columns.begin(), columns.end(),
                                                  [&conditionColumn](const Table::Column &column) {
                                                      return column.name == conditionColumn;
                                                  });

    if (conditionColumnIt == columns.end()) {
        std::cerr << "Column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return false;
    }

    int conditionColumnIndex = conditionColumnIt->index;
    bool updated = false;

    for (auto &row: tableIt->second.data) {
        if (row.size() > conditionColumnIndex && row[conditionColumnIndex] == conditionValue) {
            for (const auto &[colName, colValue]: updateValues) {
                auto updateColIt = std::ranges::find_if(columns.begin(), columns.end(),
                                                        [&colName](const Table::Column &column) {
                                                            return column.name == colName;
                                                        });

                if (updateColIt != columns.end()) {
                    int updateColumnIndex = updateColIt->index;

                    if (row.size() > updateColumnIndex) {
                        row[updateColumnIndex] = colValue;
                        updated = true;
                    }
                } else {
                    std::cerr << "Column " << colName << " does not exist in table " << tableName << "."
                              << std::endl;
                    return false;
                }
            }
        }
    }

    return updated;

}


auto Database::deleteData(const std::string &tableName, const std::string &conditionColumn,
                          const std::string &conditionValue) -> void {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &table = tableIt->second;

    // Find the index of the condition column
    auto conditionColumnIt = std::ranges::find_if(table.columns.begin(), table.columns.end(),
                                                  [&conditionColumn](const Table::Column &column) {
                                                      return column.name == conditionColumn;
                                                  });

    if (conditionColumnIt == table.columns.end()) {
        std::cerr << "Column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    int conditionColumnIndex = conditionColumnIt->index;

    // Remove rows that match the condition
    auto &data = table.data;
    auto newEnd = std::remove_if(data.begin(), data.end(),
                                 [&](const std::vector<std::string> &row) {
                                     return row.size() > conditionColumnIndex &&
                                            row[conditionColumnIndex] == conditionValue;
                                 });
    data.erase(newEnd, data.end());
}


auto Database::selectData(const std::string &tableName, const std::vector<std::string> &columns,
                          const std::string &condition) -> void {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &table = tableIt->second;

    // Create an istringstream from the condition string
    std::istringstream conditionStream(condition);

    DBQLParser dbqlParser;
    Query query;
    std::vector<Condition> conditions;
    dbqlParser.parseConditions(conditionStream, query);

    // Wyświetlanie danych
    for (const auto &row: table.data) {
        bool meetConditions = true;

        // Sprawdzanie, czy wiersz spełnia warunki
        for (const auto &cond: conditions) {
            auto columnIt = std::ranges::find_if(table.columns.begin(), table.columns.end(),
                                                 [&cond](const Table::Column &column) {
                                                     return column.name == cond.column;
                                                 });

            if (columnIt != table.columns.end()) {
                size_t columnIndex = columnIt->index;

                if (columnIndex < table.columns.size()) {
                    if (cond.op == "==" && row[columnIndex] != cond.value) {
                        meetConditions = false;
                        break;
                    }
                    // Dodaj obsługę innych operatorów porównania, np. "<", ">", itp., jeśli jest to wymagane
                }
            } else {
                std::cerr << "Column " << cond.column << " not found in specified columns." << std::endl;
                meetConditions = false;
                break;
            }
        }

        if (meetConditions) {
            for (size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                std::cout << row.at(columnIndex) << " "; // Fix: Access the element using the at() function
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

    for (const auto &tableEntry: tables) {
        const auto &tableName = tableEntry.first;
        const auto &table = tableEntry.second;

        outputFile << "Table: " << tableName << std::endl;

        // Write column names
        for (const auto &column: table.columns) {
            outputFile << column.name << ",";
        }
        outputFile << std::endl;

        // Write data rows
        for (const auto &row: table.data) {
            for (const auto &value: row) {
                outputFile << value << ",";
            }
            outputFile << std::endl;
        }

        outputFile << std::endl;
    }

    outputFile.close();
}


auto Database::loadDataFromFile(const std::string &fileName) -> void {
    std::ifstream inputFile(fileName);
    if (!inputFile) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return;
    }

    std::string line;
    std::string tableName;
    std::vector<Table::Column> columns;
    std::vector<std::vector<std::string>> data;

    DataType defaultDataType = DataType::BOOL;  // Set a default DataType

    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            if (!tableName.empty()) {
                Table loadedTable;
                loadedTable.name = tableName;
                loadedTable.columns = columns;
                loadedTable.data = std::move(data);
                tables.emplace(tableName, std::move(loadedTable));
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
                if (!columnName.empty()) {
                    columns.push_back(Table::Column(columnName, defaultDataType, static_cast<int>(columns.size())));
                }
            }
        } else {
            std::istringstream iss(line);
            std::string value;
            std::vector<std::string> rowData;
            while (std::getline(iss, value, ',')) {
                rowData.push_back(value);
            }
            data.push_back(std::move(rowData));
        }
    }

    if (!tableName.empty()) {
        Table loadedTable;
        loadedTable.name = tableName;
        loadedTable.columns = columns;
        loadedTable.data = std::move(data);
        tables.emplace(tableName, std::move(loadedTable));
    }

    inputFile.close();
}

bool Table::isValidColumnType(const Column &column) {
    for (const auto &existingColumn: columns) {
        if (existingColumn.type != column.type) {
            return false;
        }
    }
    return true;
}

void Database::executeQuery(const std::string &query) {
    DBQLParser dbqlParser;
    // Otrzymujemy elementy zapytania
    auto tableName = dbqlParser.getTableName();
    auto columns = dbqlParser.getColumns();
    auto condition = dbqlParser.getCondition();

    // Wykonanie zapytania
    selectData(tableName, columns, condition);
}

void Database::addNewColumn(const std::string &tableName, const std::string &columnName, DataType columnType) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Sprawdzenie, czy kolumna o podanej nazwie już istnieje
    auto &columns = tableIt->second.columns;
    if (std::ranges::find_if(columns.begin(), columns.end(),
                             [&columnName](const auto &col) { return col.name == columnName; }) != columns.end()) {
        std::cerr << "Column " << columnName << " already exists in table " << tableName << "." << std::endl;
        return;
    }

    // Dodanie nowej kolumny do istniejącej tabeli
    int columnIndex = static_cast<int>(columns.size());
    columns.push_back(Table::Column{columnName, columnType, columnIndex});

    // Zainicjowanie pustych wartości dla nowej kolumny we wszystkich wierszach
    for (auto &row: tableIt->second.data) {
        row.push_back("");
    }


}