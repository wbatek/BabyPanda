#ifndef ABSTRACTPROGRAMMINGPROJECT_DATAFRAME_H
#define ABSTRACTPROGRAMMINGPROJECT_DATAFRAME_H

#include <iostream>
#include "column.h"
#include <tuple>
#include <map>
#include <any>

using ColumnType = std::variant<int, double, bool, std::string>;

class DataFrame {
private:
    std::string name;
    std::map<std::string, Column<ColumnType>> columns;
    std::map<std::string, size_t> columnIndex;

public:
    // BASIC HANDLING
    std::string getName() const;
    std::vector<Column<ColumnType>> getColumns() const;
    void setName(const std::string& columnName);
    size_t numberOfColumns() const;
    size_t numberOfRows() const;
    std::pair<size_t, size_t> shape() const;
    bool isEmpty() const;
    void print() const;
    std::vector<std::string> columnNames() const;

    // DATA MANIPULATION
    Column<ColumnType>& getColumn(size_t index);
    Column<ColumnType>& getColumn(const std::string& n);
    template<class T> void addColumn(const Column<T>& column);
    void addColumn(const std::string& name);
    void addColumn();
    void addRow(const std::vector<std::optional<ColumnType>>& row);
    void removeRow(size_t index);
    Column<ColumnType> removeColumn(const std::string& columnName);
    Column<ColumnType> removeColumn(size_t index);
    DataFrame selectColumns(const std::vector<std::string>& columnNames);
    DataFrame selectColumns(const std::vector<size_t>& indexes);
    DataFrame filterRows(const std::function<bool(const std::vector<std::optional<ColumnType>>&)>& pred) const;

    // STATISTICS
    std::map<std::string, std::map<std::string, ColumnType>> describe() const;
    std::map<std::string, std::map<std::string, ColumnType>> aggregate(const std::vector<std::string>& operations) const;
    std::map<std::string, std::map<std::string, ColumnType>> min() const;
    std::map<std::string, std::map<std::string, ColumnType>> max() const;
    std::map<std::string, std::map<std::string, ColumnType>> mean() const;
    std::map<std::string, std::map<std::string, ColumnType>> var() const;
    std::map<std::string, std::map<std::string, ColumnType>> std() const;
    std::map<std::string, std::map<std::string, ColumnType>> median() const;
    std::map<std::string, std::map<std::string, ColumnType>> skewness() const;

    // NULL-HANDLING
    void fillNullWithDefault();
    void fillNull(std::vector<ColumnType>& values);

    // SORTING
    DataFrame sortBy(const std::string& columnName, bool ascending = true) const;

    // FILES
    static DataFrame readCSV(const std::string& filePath, const std::string& separator = ",", bool hasHeaderLine = true);
    void saveCSV(const std::string& filePath, const std::string& separator = ",", bool saveHeaderLine = true);
};

#endif //ABSTRACTPROGRAMMINGPROJECT_DATAFRAME_H
