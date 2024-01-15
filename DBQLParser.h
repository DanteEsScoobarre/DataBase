#ifndef DATABASE_DBQLPARSER_H
#define DATABASE_DBQLPARSER_H


#include "PreRequistion.h"
#include "Database.h"

struct Condition {
    std::string column;
    std::string op;
    std::string value;

    Condition(const std::string &col, const std::string &operation, const std::string &val)
            : column(col), op(operation), value(val) {}
};


struct Query {
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
    std::vector<std::string> logicalOperators;
    std::vector<std::string> groupByColumns;
    std::vector<Condition> havingConditions;
};

struct ColumnDefinition {
    std::string name;
    std::string dataType;
};

struct TableDefinition {
    std::string tableName;
    std::vector<ColumnDefinition> columns;
};


class DBQLParser {
public:
    DBQLParser() = default;
    // Metody dostępu do elementów zapytania
    auto getTableName() const -> std::string;

    auto getColumns() const -> std::vector<std::string>;

    auto getCondition() const -> std::string;

    auto parseSelectCommand(const std::string &command, Database& db) -> void;

    auto parseColumns(std::istringstream &iss, Query &query) -> void;

    auto parseHaving(std::istringstream& iss, Query& query) -> void;

    auto parseGroupBy(std::istringstream &iss, Query &query) -> void;


    auto parseConditions(std::istringstream &iss, Query &query) -> void;

    auto isValidOperator(const std::string &op) -> bool;
    auto parseCreateTableCommand(const std::string &command) -> TableDefinition;
    auto executeCreateTable(const std::string &command, Database &database) -> void;

private:
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;



};


#endif //DATABASE_DBQLPARSER_H
