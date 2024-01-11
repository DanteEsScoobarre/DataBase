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

void Database::addColumn(const std::string &tableName, const Table::Column &column) {
    auto table = tables.find(tableName);
    if (table == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    if (std::find_if(table->second.columns.begin(), table->second.columns.end(),
                     [&column](const Table::Column &existingColumn) { return existingColumn.name == column.name; }) !=
        table->second.columns.end()) {
        std::cerr << "Column " << column.name << " already exists in table " << tableName << "." << std::endl;
        return;
    }

    if (!table->second.isValidColumnType(column)) {
        std::cerr << "Invalid type for column " << column.name << " in table " << tableName << "." << std::endl;
        return;
    }

    // Utwórz nowy obiekt Column używając konstruktora
    Table::Column newColumn(column.name, column.type, static_cast<int>(table->second.columns.size()));
    table->second.columns.push_back(newColumn);
}


void Database::removeColumn(const std::string &tableName, const std::string &columnName) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &columns = tableIt->second.columns;
    auto columnIt = std::find_if(columns.begin(), columns.end(),
                                 [&columnName](const Table::Column &column) { return column.name == columnName; });

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


void Database::insertData(const std::string &tableName, const std::map<std::string, std::string> &rowData) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    // Initialize a new row with empty values
    std::vector<std::string> newRow(tableIt->second.columns.size(), "");

    // Fill in the values from the provided rowData
    for (const auto &col: rowData) {
        auto colIt = std::find_if(tableIt->second.columns.begin(), tableIt->second.columns.end(),
                                  [&col](const Table::Column &column) { return column.name == col.first; });

        if (colIt != tableIt->second.columns.end()) {
            newRow[colIt->index] = col.second;
        } else {
            std::cerr << "Column " << col.first << " does not exist in table " << tableName << "." << std::endl;
            return;
        }
    }

    // Add the new row to the data vector
    tableIt->second.data.push_back(newRow);
}


void Database::updateData(const std::string &tableName, const std::map<std::string, std::string> &updateValues,
                          const std::string &conditionColumn, const std::string &conditionValue) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &columns = tableIt->second.columns;

    // Sprawdzanie, czy kolumna warunku istnieje i uzyskanie jej indeksu
    auto conditionColumnIt = std::find_if(columns.begin(), columns.end(),
                                          [&conditionColumn](const Table::Column &column) {
                                              return column.name == conditionColumn;
                                          });

    if (conditionColumnIt == columns.end()) {
        std::cerr << "Column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    int conditionColumnIndex = conditionColumnIt->index;

    // Przechodzenie przez wszystkie wiersze i aktualizacja danych
    for (auto &row: tableIt->second.data) {
        if (row.size() > conditionColumnIndex && row[conditionColumnIndex] == conditionValue) {
            for (const auto &colVal: updateValues) {
                auto updateColIt = std::find_if(columns.begin(), columns.end(),
                                                [&colVal](const Table::Column &column) {
                                                    return column.name == colVal.first;
                                                });

                if (updateColIt != columns.end()) {
                    int updateColumnIndex = updateColIt->index;

                    if (row.size() > updateColumnIndex) {
                        row[updateColumnIndex] = colVal.second;
                    }
                } else {
                    std::cerr << "Column " << colVal.first << " does not exist in table " << tableName << "."
                              << std::endl;
                    return;
                }
            }
        }
    }
}


void Database::deleteData(const std::string &tableName, const std::string &conditionColumn,
                          const std::string &conditionValue) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &columns = tableIt->second.columns;

    auto conditionColumnIt = std::find_if(columns.begin(), columns.end(),
                                          [&conditionColumn](const Table::Column &column) {
                                              return column.name == conditionColumn;
                                          });

    if (conditionColumnIt == columns.end()) {
        std::cerr << "Column " << conditionColumn << " does not exist in table " << tableName << "." << std::endl;
        return;
    }

    int conditionColumnIndex = conditionColumnIt->index;

    auto &data = tableIt->second.data;
    std::vector<std::vector<std::string>> newData;

    for (const auto &row: data) {
        if (row.size() > conditionColumnIndex && row[conditionColumnIndex] != conditionValue) {
            newData.push_back(row);
        }
    }

    data = std::move(newData);
}


void Database::selectData(const std::string &tableName, const std::vector<std::string> &columns,
                          const std::string &condition) {
    auto tableIt = tables.find(tableName);
    if (tableIt == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }

    auto &table = tableIt->second;

    // Parsowanie warunku (na razie bardzo proste)
    std::vector<Condition> conditions;
    DBQLParser::parseConditions(condition, conditions);

    // Wyświetlanie danych
    for (const auto &row: table.data) {
        bool meetConditions = true;

        // Sprawdzanie, czy wiersz spełnia warunki
        for (const auto &cond: conditions) {
            auto columnIt = std::find_if(table.columns.begin(), table.columns.end(),
                                         [&cond](const Table::Column &column) { return column.name == cond.column; });

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


/*void Database::loadDataFromFile(const std::string &fileName) {
    std::ifstream inputFile(fileName);
    if (!inputFile) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return;
    }

    std::string line;
    std::string tableName;
    std::vector<Table::Column> columns;
    std::vector<std::vector<std::string>> data;

    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            if (!tableName.empty()) {
                Table loadedTable;
                loadedTable.name = tableName;
                loadedTable.columns = columns;
                loadedTable.data = std::move(data);
                tables.emplace(tableName, loadedTable);
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
                columns.push_back({columnName});
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
        tables.emplace(tableName, loadedTable);
    }

    inputFile.close();
}
*/

bool Table::Column::isValidType(const std::string &value) const {
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

int Table::getConditionColumnIndex(const std::string &conditionColumn) {
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].name == conditionColumn) {
            return static_cast<int>(i);
        }
    }

    std::cerr << "Condition column " << conditionColumn << " does not exist in table " << name << "." << std::endl;
    return -1;
}


bool Database::tableExists(const std::string &tableName) const {
    return tables.find(tableName) != tables.end();
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
    DBQLParser dbqlParser(query);


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
    if (std::find_if(columns.begin(), columns.end(),
                     [&columnName](const auto &col) { return col.name == columnName; }) != columns.end()) {
        std::cerr << "Column " << columnName << " already exists in table " << tableName << "." << std::endl;
        return;
    }

    // Dodanie nowej kolumny do istniejącej tabeli
    int columnIndex = static_cast<int>(columns.size());
    columns.push_back(Table::Column{columnName, columnType, columnIndex});

    // Zainicjowanie pustych wartości dla nowej kolumny we wszystkich wierszach
    for (auto &row: tableIt->second.data) {
        for (size_t i = 0; i < tableIt->second.columns.size(); ++i) {
            row.push_back("");  // Add an empty string for each column
        }
    }


}