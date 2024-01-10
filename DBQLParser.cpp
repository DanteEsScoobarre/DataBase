#include "DBQLParser.h"

std::string DBQLParser::getTableName() const {
    return std::string();
}

std::vector<std::string> DBQLParser::getColumns() const {
    return std::vector<std::string>();
}

std::string DBQLParser::getCondition() const {
    return std::string();
}

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