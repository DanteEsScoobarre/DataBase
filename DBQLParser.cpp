#include "DBQLParser.h"

/**
 * @brief Get the table name.
 *
 * @return std::string The table name.
 */
std::string DBQLParser::getTableName() const {
    return tableName;
}

/**
 * @brief Returns the vector of strings representing the columns.
 *
 * @return std::vector<std::string> The vector of strings representing the columns.
 */
std::vector<std::string> DBQLParser::getColumns() const {
    return columns;
}

/**
 * @brief Returns the condition as a string.
 *
 * If the conditions vector is not empty, this function returns the first condition's column, operator, and value concatenated as a string.
 * If the conditions vector is empty, an empty string is returned.
 *
 * @return std::string The condition as a string.
 */
std::string DBQLParser::getCondition() const {
    if (!conditions.empty()) {
        return conditions[0].column + conditions[0].op + conditions[0].value;
    }
    return "";
}

/**
 * @brief Parses a SQL query and extracts the SELECT columns, table name, and WHERE conditions.
 *
 * @param query The SQL query to parse.
 */
void DBQLParser::parse(const std::string &query) {
    std::istringstream iss(query);
    std::string token;
    bool isSelecting = false, isFrom = false, isWhere = false;
    Condition currentCondition;
    bool expectLogicalOperator = false;

    while (iss >> token) {
        if (token == "SELECT") {
            isSelecting = true;
            isFrom = isWhere = false;
        } else if (token == "FROM") {
            isFrom = true;
            isSelecting = isWhere = false;
        } else if (token == "WHERE") {
            isWhere = true;
            isSelecting = isFrom = false;
            currentCondition = Condition();
        } else if (token == "AND" || token == "OR") {
            if (expectLogicalOperator) {
                currentCondition.logicalOperator = token;
            } else {
                std::cerr << "Unexpected logical operator " << token << "." << std::endl;
                return;
            }
        } else {
            if (isSelecting) {
                if (token.back() == ',') {
                    token.pop_back(); // Usuwamy przecinek
                }
                columns.push_back(token);
            } else if (isFrom) {
                tableName = token;
            } else if (isWhere) {
                if (currentCondition.column.empty()) {
                    currentCondition.column = token;
                } else if (currentCondition.op.empty()) {
                    currentCondition.op = token;
                } else {
                    currentCondition.value = token;
                    conditions.push_back(currentCondition);
                    currentCondition = Condition();
                    expectLogicalOperator = true;
                }
            }
        }
    }
}
/**
 * @brief Parses the given condition string and populates the vector of conditions.
 *
 * This function parses the condition string and extracts individual conditions, which are then
 * stored in the provided vector. Each condition consists of a column, operator, and value.
 * The condition string should be in the format "column operator value", where the operator can be
 * either "AND" or "OR". Multiple conditions can be separated by spaces.
 *
 * @param condition The condition string to parse.
 * @param conditions The vector to store the parsed conditions.
 */
void DBQLParser::parseConditions(const std::string& condition, std::vector<Condition>& conditions) {
    std::istringstream iss(condition);
    std::string token;
    Condition currentCondition;

    while (iss >> token) {
        if (token == "AND" || token == "OR") {
            if (!currentCondition.column.empty()) {
                conditions.push_back(currentCondition);
                currentCondition = Condition();
            }
            currentCondition.logicalOperator = token;
        } else {
            if (currentCondition.column.empty()) {
                currentCondition.column = token;
            } else if (currentCondition.op.empty()) {
                currentCondition.op = token;
            } else {
                currentCondition.value = token;
                conditions.push_back(currentCondition);
                currentCondition = Condition();
            }
        }
    }
    if (!currentCondition.column.empty()) {
        conditions.push_back(currentCondition);
    }
}