#include "DBQLParser.h"


auto DBQLParser::getTableName() const -> std::string {
    return tableName;
}

auto DBQLParser::getColumns() const -> std::vector<std::string> {
    return columns;
}

auto DBQLParser::getCondition() const -> std::string {
    if (!conditions.empty()) {
        return conditions[0].column + conditions[0].op + conditions[0].value;
    }
    return "";
}

auto DBQLParser::parseSelectCommand(const std::string &command, Database &db) -> void {
    std::istringstream iss(command);
    std::string token;
    Query query;

    while (iss >> token) {
        if (token == "select") {
            parseColumns(iss, query);
        } else if (token == "from") {
            iss >> query.tableName;
        } else if (token == "where") {
            parseConditions(iss, query);
        } else if (token == "having") {
            parseHaving(iss, query);
        } else if (token == "group" && iss.peek() == ' ') {
            std::string nextToken;
            iss >> nextToken;
            if (nextToken == "by") {
                parseGroupBy(iss, query);
            }

            std::string conditionStr;
            for (const auto &cond: query.conditions) {
                if (!conditionStr.empty()) conditionStr += " AND ";
                conditionStr += cond.column + cond.op + cond.value;
            }
            db.selectData(query.tableName, query.columns, conditionStr);
        }
    }
}


auto DBQLParser::parseColumns(std::istringstream &iss, Query &query) -> void {
    std::string column;
    while (iss >> column && column != "from") {
        query.columns.push_back(column);
        if (iss.peek() == ',') {
            iss.ignore();
        }
    }
}

auto DBQLParser::parseConditions(std::istringstream &iss, Query &query) -> void {
    std::string column, op, value;
    while (iss >> column >> op >> value) {
        if (!isValidOperator(op)) {
            std::cerr << "Invalid operator: " << op << std::endl;
            continue;
        }
        query.conditions.push_back(Condition(column, op, value));
        if (iss.peek() == ',' || iss.peek() == 'and') {
            iss.ignore();
        }
    }
}

auto DBQLParser::isValidOperator(const std::string &op) -> bool {
    static const auto validOps = std::vector<std::string>{"=", "<", ">", "!=", "<=", ">="};
    return std::find(validOps.begin(), validOps.end(), op) != validOps.end();
}

auto DBQLParser::parseHaving(std::istringstream &iss, Query &query) -> void {
    std::string column, op, value, logicalOperator;
    while (iss >> column >> op >> value) {
        if (!isValidOperator(op)) {
            std::cerr << "Invalid operator in HAVING clause: " << op << std::endl;
            continue; // Skip this condition and proceed to the next
        }
        query.havingConditions.push_back(Condition(column, op, value));
        auto nextChar = iss.peek();
        if (nextChar == ',' || nextChar == 'and' || nextChar == 'or') {
            if (nextChar != ',') {
                iss >> logicalOperator;
            }
            iss.ignore();
        }
    }
}


auto DBQLParser::parseGroupBy(std::istringstream &iss, Query &query) -> void {
    std::string column;
    while (iss >> column) {
        query.groupByColumns.push_back(column);
        if (iss.peek() == ',') iss.ignore();
    }
}

auto DBQLParser::parseCreateTableCommand(const std::string &command) -> TableDefinition {
    std::istringstream iss(command);
    std::string token, tableName;
    TableDefinition tableDef;

    iss >> token; // This should be "create"
    if (token != "create") {
        throw std::runtime_error("Expected 'create' command");
    }

    iss >> tableName; // Extract table name
    tableDef.tableName = tableName;

    // Parse column definitions
    std::string columnName, dataType;
    while (iss >> columnName >> dataType) {
        tableDef.columns.push_back({columnName, dataType});
        if (iss.peek() == ',') {
            iss.ignore();
        }
    }

    return tableDef;
}

auto DBQLParser::executeCreateTable(const std::string& command, Database& database) -> void{
    try {
        TableDefinition tableDef = parseCreateTableCommand(command);
        database.createTable(tableDef.tableName, tableDef.columns);
    } catch (const std::exception& e) {
        std::cerr << "Error creating table: " << e.what() << std::endl;
    }
}
