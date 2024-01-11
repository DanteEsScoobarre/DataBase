#include "Database.h"
#include "DBQLParser.h"


Database::Database() {
}


/**
 * Creates a new table in the database with the specified table name.
 * If a table with the same name already exists, an error message is printed and no action is taken.
 *
 * @param tableName The name of the table to be created.
 */
void Database::createTable(const std::string &tableName) {
    if (tables.find(tableName) != tables.end()) {
        std::cerr << "Table " << tableName << " already exists." << std::endl;
        return;
    }
    tables[tableName] = Table{tableName};
}


/**
 * @brief Drops a table from the database.
 *
 * This function removes the specified table from the database. If the table does not exist,
 * an error message is printed to the standard error stream.
 *
 * @param tableName The name of the table to be dropped.
 */
void Database::dropTable(const std::string &tableName) {
    if (tables.find(tableName) == tables.end()) {
        std::cerr << "Table " << tableName << " does not exist." << std::endl;
        return;
    }
    tables.erase(tableName);

}

/**
 * Adds a column to the specified table in the database.
 *
 * @param tableName The name of the table.
 * @param column The column to be added.
 */
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


/**
 * Removes a column from a table in the database.
 *
 * @param tableName The name of the table.
 * @param columnName The name of the column to be removed.
 */
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

/**
 * @brief Updates data in a specified table based on a given condition.
 *
 * This function updates the values of specified columns in a table based on a given condition.
 * It searches for the specified table in the database, checks if the condition column exists,
 * and then iterates through each row in the table's data. If a row satisfies the condition,
 * the function updates the specified columns with the provided values.
 *
 * @param tableName The name of the table to update.
 * @param updateValues A map containing the column names and their corresponding updated values.
 * @param conditionColumn The name of the column used as the condition for updating the data.
 * @param conditionValue The value that the condition column must match for a row to be updated.
 */

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



/**
 * @brief Updates data in a specified table based on a given condition.
 *
 * This function updates the values of specified columns in a table based on a given condition.
 * It searches for the specified table in the database and checks if the condition column exists.
 * If the table or condition column does not exist, an error message is printed and the function returns.
 * If the condition column exists, it iterates through all the rows in the table and updates the values
 * of the specified columns for rows that match the condition.
 *
 * @param tableName The name of the table to update.
 * @param updateValues A map containing the column names and their corresponding updated values.
 * @param conditionColumn The name of the column used as the condition for updating the data.
 * @param conditionValue The value that the condition column must match for a row to be updated.
 */
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


/**
 * Deletes data from a specified table based on a condition.
 *
 * @param tableName The name of the table from which to delete data.
 * @param conditionColumn The name of the column used as the condition for deletion.
 * @param conditionValue The value that must be matched in the condition column for a row to be deleted.
 */
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


/**
 * Selects data from a specified table based on the given conditions and displays the selected columns.
 *
 * @param tableName The name of the table to select data from.
 * @param columns The vector of column names to be displayed.
 * @param condition The condition to filter the rows.
 */
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


/**
 * Saves the contents of the database to a file.
 *
 * @param fileName The name of the file to save the database to.
 */
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

/**
 * Loads data from a file into the database.
 *
 * @param fileName The name of the file to load data from.
 */

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

/**
 * @brief Checks if a given value is a valid type for the column.
 *
 * @param value The value to be checked.
 * @return true if the value is a valid type, false otherwise.
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

/**
 * Returns the index of the column with the specified condition column name.
 *
 * @param conditionColumn The name of the condition column.
 * @return The index of the condition column, or -1 if it does not exist.
 */
int Table::getConditionColumnIndex(const std::string &conditionColumn) {
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].name == conditionColumn) {
            return static_cast<int>(i);
        }
    }

    std::cerr << "Condition column " << conditionColumn << " does not exist in table " << name << "." << std::endl;
    return -1;
}


/**
 * Checks if a table exists in the database.
 *
 * @param tableName The name of the table to check.
 * @return True if the table exists, false otherwise.
 */
bool Database::tableExists(const std::string &tableName) const {
    return tables.find(tableName) != tables.end();
}

/**
 * Checks if the given column type is valid for the table.
 *
 * @param column The column to check.
 * @return True if the column type is valid, false otherwise.
 */
bool Table::isValidColumnType(const Column &column) {
    for (const auto &existingColumn: columns) {
        if (existingColumn.type != column.type) {
            return false;
        }
    }
    return true;
}


/**
 * Executes a database query.
 *
 * @param query The query to be executed.
 */
void Database::executeQuery(const std::string &query) {
    DBQLParser dbqlParser(query);


    // Otrzymujemy elementy zapytania
    auto tableName = dbqlParser.getTableName();
    auto columns = dbqlParser.getColumns();
    auto condition = dbqlParser.getCondition();

    // Wykonanie zapytania
    selectData(tableName, columns, condition);
}

/**
 * @brief Adds a new column to the specified table in the database.
 *
 * This function checks if the table exists and if the column with the given name already exists in the table.
 * If the table or column already exists, an error message is printed and the function returns.
 * Otherwise, a new column is added to the table with the specified name and data type.
 * Empty values are initialized for the new column in all rows of the table.
 *
 * @param tableName The name of the table to add the column to.
 * @param columnName The name of the new column.
 * @param columnType The data type of the new column.
 */
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