#ifndef DATABASE_DBQLPARSER_H
#define DATABASE_DBQLPARSER_H
#include "PreRequistion.h"
struct Condition {
    std::string column;
    std::string op;
    std::string value;
};

struct Query {
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
    std::vector<std::string> logicalOperators; // AND, OR
};

class DBQLParser {
public:
    DBQLParser(const std::string &query) {
        parse(query);
    }

    // Metody dostępu do elementów zapytania
    std::string getTableName() const;

    std::vector<std::string> getColumns() const;

    std::string getCondition() const;

private:
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;


    void parse(const std::string &query);

};


#endif //DATABASE_DBQLPARSER_H
