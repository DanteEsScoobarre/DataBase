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

class DBQLParser {
public:
    DBQLParser(const std::string &query) {
        parse(query);
    }

    // Metody dostępu do elementów zapytania
    auto getTableName() const -> std::string;

    auto getColumns() const -> std::vector<std::string>;

    auto getCondition() const -> std::string;

    auto parseSelectCommand(const std::string &command, Database& db) -> void;

    auto parseColumns(std::istringstream &iss, Query &query) -> void;

    auto parseHaving(std::istringstream& iss, Query& query) -> void;

    auto parseGroupBy(std::istringstream &iss, Query &query) -> void;

    void parse(const std::string &query);

    auto parseConditions(std::istringstream &iss, Query &query) -> void;

    auto isValidOperator(const std::string &op) -> bool;

private:
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;



};


#endif //DATABASE_DBQLPARSER_H
