#include "include/dataframe.h"
#include "include/exceptions.h"
#include "include/column.h"
#include "src/column.cpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <any>

// BASIC HANDLING

std::string DataFrame::getName() const {
    return this->name;
}

std::vector<Column<ColumnType>> DataFrame::getColumns() const {
    std::vector<Column<ColumnType>> columnVector;

    for(const auto& pair : this->columns) {
        columnVector.push_back(pair.second);
    }

    return columnVector;
}

void DataFrame::setName(const std::string& columnName) {
    this->name = columnName;
}

size_t DataFrame::numberOfColumns() const {
    return this->columns.size();
}

size_t DataFrame::numberOfRows() const {
    return this->columns.empty() ? 0 : this->columns.begin()->second.size();
}

std::pair<size_t, size_t> DataFrame::shape() const {
    return std::make_pair(this->numberOfColumns(), this->numberOfRows());
}

bool DataFrame::isEmpty() const {
    return this->numberOfRows() == 0;
}

//void DataFrame::print() const {
//    std::vector<size_t> columnWidths(columns.size());
//    for (size_t i = 0; i < columns.size(); ++i) {
//        columnWidths[i] = columns[i]->getWidth();
//    }
//
//    for (size_t i = 0; i < columns.size(); ++i) {
//        std::cout << "|" << std::setw(columnWidths[i]) << columns[i]->getName();
//    }
//    std::cout << "|" << std::endl;
//
//    for (size_t i = 0; i < columns.size(); ++i) {
//        std::cout << "+" << std::string(columnWidths[i], '-');
//    }
//    std::cout << "+" << std::endl;
//
//    // Print rows
//    size_t maxRows = numberOfRows();
//    for (size_t rowIndex = 0; rowIndex < maxRows; ++rowIndex) {
//        for (size_t colIndex = 0; colIndex < columns.size(); ++colIndex) {
//            auto& column = columns[colIndex];
//            auto value = (*column)[rowIndex];
////            if (value.has_value()) {
////                std::cout << "|" << std::setw(columnWidths[colIndex]) << value.value();
////            } else {
////                std::cout << "|" << std::setw(columnWidths[colIndex]) << "null";
////            }
//            column->printElement(value, std::cout, columnWidths[colIndex]);
//        }
//        std::cout << "|" << std::endl;
//    }
//}


std::vector<std::string> DataFrame::columnNames() const {
    std::vector<std::string> r;
    r.reserve(this->columns.size());
    for(auto& col : this->getColumns()) {
        r.push_back(col.getName());
    }
    return r;
}

// END BASIC HANDLING

// DATA MANIPULATION

Column<ColumnType>& DataFrame::getColumn(size_t index) {
    for (const auto& pair : columnIndex) {
        if (pair.second == index) {
            const std::string& columnName = pair.first;
            return columns.at(columnName);
        }
    }

    throw std::out_of_range("Index not found in columnIndex");
}

Column<ColumnType>& DataFrame::getColumn(const std::string &n) {
    if (columnIndex.find(n) == columnIndex.end()) {
        throw std::runtime_error("Column not found");
    }
    return columns.at(n);
}

template<class T>
void DataFrame::addColumn(const Column<T>& column) {
    if (this->numberOfColumns() != 0) {
        if (column.size() != this->columns.begin()->second.size()) {
            throw InvalidSizeException();
        }
    }
    Column<ColumnType> newColumn(column.getName());
    for (const auto& value : column.getOptionalValues()) {
        if (value.has_value()) {
            newColumn.add(std::optional<ColumnType>(*value));
        } else {
            newColumn.add(std::nullopt);
        }
    }
    const std::string& columnName = newColumn.getName();
    if (this->columns.find(columnName) != this->columns.end()) {
        throw std::runtime_error("Column with the same name already exists");
    }
    this->columns[columnName] = newColumn;
    this->columnIndex[columnName] = this->columns.size() - 1;
}

void DataFrame::addColumn(const std::string& columnName) {
    columns[columnName] = Column<ColumnType>(columnName);
    columnIndex[columnName] = this->columns.size() - 1;
}

void DataFrame::addColumn() {
    Column<ColumnType> newColumn = Column<ColumnType>();
    columns[newColumn.getName()] = newColumn;
    columnIndex[newColumn.getName()] = this->columns.size() - 1;
}

void DataFrame::addRow(const std::vector<std::optional<ColumnType>>& row) {
    if(row.size() != this->numberOfColumns()) {
        std::cout << row.size() << " " << this->numberOfColumns() << std::endl;
        throw std::invalid_argument("Row size does not match number of columns");
    }

    size_t i = 0;
    for(auto& colPair : columns) {
        Column<ColumnType>& column = colPair.second;

        if(i < row.size()) {
            const auto& cellValue = row[i];
            if(cellValue.has_value()) {
                bool typeMismatch = false;
                try {
                    if(column.getValues().empty()) {
                        typeMismatch = false;
                    }
                    else {
                        auto firstNonNull = column.getValues()[0];
                        std::visit([&](auto&& value) {
                            using T = std::decay_t<decltype(value)>;
                            try {
                                (void)std::get<T>(*cellValue);

                                typeMismatch = false;
                            } catch (const std::bad_variant_access& ex) {
                                typeMismatch = true;
                            }
                        }, firstNonNull);
                    }
                }
                catch (...) {
                    typeMismatch = false;
                }
                if (typeMismatch) {
                    column.add(std::nullopt);
                }
                else {
                    column.add(cellValue);
                }
            }
            else {
                column.add(std::nullopt);
            }
        }
        i++;
    }
}

void DataFrame::removeRow(size_t index) {
    if(index >= this->numberOfRows()) {
        throw InvalidIndexException();
    }

    for(auto& colPair : this->columns) {
        auto& col = colPair.second;
        col.removeAtFromRow(index);
    }
}

Column<ColumnType> DataFrame::removeColumn(const std::string& columnName) {
    if(this->columns.find(columnName) == this->columns.end()) {
        throw InvalidNameException();
    }
    Column<ColumnType> removedColumn = this->columns.at(columnName);
    this->columns.erase(columnName);

    auto it = columnIndex.find(columnName);
    size_t removedIndex = it->second;
    columnIndex.erase(it);

    for (auto& pair : columnIndex) {
        if (pair.second > removedIndex) {
            pair.second--;
        }
    }
    return removedColumn;
}

Column<ColumnType> DataFrame::removeColumn(size_t index) {
    if(index >= this->numberOfColumns()) {
        throw InvalidIndexException();
    }
    std::string columnName;
    for (const auto& pair : columnIndex) {
        if (pair.second == index) {
            columnName = pair.first;
            break;
        }
    }
    return this->removeColumn(columnName);
}

void DataFrame::print() const {
    size_t maxWidthFinal = 0;

    for (const auto& colPair : columns) {
        const std::string& columnName = colPair.first;
        const Column<ColumnType>& column = colPair.second;

        size_t maxWidth = columnName.size();

        for (const auto& val : column.getOptionalValues()) {
            size_t valueWidth = 0;

            if (val.has_value()) {
                std::visit([&valueWidth](auto&& v) {
                    std::ostringstream oss;
                    oss << v;
                    valueWidth = oss.str().size();
                }, val.value());
            } else {
                valueWidth = std::string("null").size();
            }

            maxWidth = std::max(maxWidth, valueWidth);
        }

        maxWidthFinal = std::max(maxWidth, maxWidthFinal);
    }

    for (const auto& colPair : columns) {
        const std::string& columnName = colPair.first;
        const Column<ColumnType>& column = colPair.second;

        std::cout << std::left << std::setw(maxWidthFinal) << columnName << ": ";

        for (const auto& val : column.getOptionalValues()) {
            if (val.has_value()) {
                std::visit([&](auto&& v) {
                    std::cout << std::setw(maxWidthFinal) << v << " ";
                }, val.value());
            } else {
                std::cout << std::setw(maxWidthFinal) << "null";
            }
        }

        std::cout << std::endl;
    }
}



DataFrame DataFrame::selectColumns(const std::vector<std::string> &columnNames) {
    DataFrame selectedDf;
    for(const auto& colName : columnNames) {
        if (this->columns.find(colName) == this->columns.end()) {
            throw InvalidNameException();
        }
    }
    for(const auto& colName : columnNames) {
        selectedDf.addColumn(getColumn(colName));
    }
    return selectedDf;
}

DataFrame DataFrame::selectColumns(const std::vector<size_t> &indexes) {
    DataFrame selectedDf;
    for(const auto& idx : indexes) {
        if (idx >= this->numberOfColumns()) {
            throw InvalidIndexException();
        }
    }
    for(const auto& idx : indexes) {
        for (const auto& pair : columnIndex) {
            if (pair.second == idx) {
                selectedDf.addColumn(getColumn(pair.first));
            }
        }
    }
    return selectedDf;
}

DataFrame DataFrame::filterRows(const std::function<bool(const std::vector<std::optional<ColumnType>>&)>& pred) const {
    DataFrame filteredDf;
    size_t numberOfRows = this->numberOfRows();
    for(size_t rowIdx = 0; rowIdx < numberOfRows; rowIdx++) {
        std::vector<std::optional<ColumnType>> row;
        for (const auto& colPair : this->columns) {
            const Column<ColumnType>& col = colPair.second;
            if (rowIdx < col.size()) {
                row.push_back(col[rowIdx]);
            } else {
                row.push_back(std::nullopt);
            }
        }

        if (pred(row)) {
            filteredDf.addRow(row);
        }
    }
    return filteredDf;
}

// STATISTICS

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::aggregate(const std::vector<std::string>& operations) const {
    std::map<std::string, std::map<std::string, ColumnType>> results;
    for (const auto& [colName, column] : columns) {
        std::map<std::string, ColumnType> columnResults;
        for (const auto& op : operations) {
            if (op == "min") {
                columnResults["min"] = std::visit([](const auto& value) -> ColumnType {
                    return value;
                }, column.min());
            } else if (op == "max") {
                columnResults["max"] = std::visit([](const auto& value) -> ColumnType {
                    return value;
                }, column.max());
            } else if (op == "mean") {
                columnResults["mean"] = column.mean();
            } else if (op == "std") {
                columnResults["std"] = column.std();
            } else if (op == "var") {
                columnResults["var"] = column.var();
            } else if (op == "countNull") {
                columnResults["countNull"] = column.countNull();
            } else if (op == "nunique") {
                columnResults["nunique"] = column.countDistinct();
            } else if (op == "median") {
                columnResults["median"] = column.median();
            } else {
                throw std::invalid_argument("Unsupported operation: " + op);
            }
        }
        results[colName] = columnResults;
    }
    return results;
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::describe() const {
    return this->aggregate({"mean", "std", "var", "min", "max", "median"});
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::max() const {
    return this->aggregate({"max"});
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::min() const {
    return this->aggregate({"min"});
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::mean() const {
    return this->aggregate({"mean"});
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::std() const {
    return this->aggregate({"std"});
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::var() const {
    return this->aggregate({"var"});
}

std::map<std::string, std::map<std::string, ColumnType>> DataFrame::median() const {
    return this->aggregate({"median"});
}

// NULL-HANDLING

void DataFrame::fillNullWithDefault() {
    for(auto& colPair : this->columns) {
        colPair.second.fillNull();
    }
}

void DataFrame::fillNull(std::vector<ColumnType> &values) {
    if(values.size() != this->numberOfColumns()) {
        throw InvalidSizeException();
    }
    int i = 0;
    for(auto& colPair : this->columns) {
        colPair.second.fillNull(values[i]);
        i++;
    }
}

// SORTING

DataFrame DataFrame::sortBy(const std::string &columnName, bool ascending) const {
    auto colIter = columns.find(columnName);
    if (colIter == columns.end()) {
        throw std::invalid_argument("Column " + columnName + " does not exist in the DataFrame.");
    }
    const auto& sortColumn = colIter->second;
    size_t nRows = numberOfRows();

    std::vector<size_t> rowIndices(nRows);
    for (size_t i = 0; i < nRows; ++i) {
        rowIndices[i] = i;
    }

    std::sort(rowIndices.begin(), rowIndices.end(), [&](size_t i, size_t j) {
        const auto& valueA = sortColumn[i];
        const auto& valueB = sortColumn[j];
        if (!valueA.has_value() || !valueB.has_value()) {
            return ascending ? valueA.has_value() : valueB.has_value();
        }
        if (ascending) {
            return valueA < valueB;
        } else {
            return valueA > valueB;
        }
    });

    DataFrame sortedDf;
    sortedDf.name = this->name;
    for (const auto& [colName, column] : columns) {
        Column<ColumnType> sortedColumn(column.getName());
        for (size_t index : rowIndices) {
            sortedColumn.add(column[index]);
        }
        sortedDf.columns[colName] = sortedColumn;
    }

    return sortedDf;
}

// FILES

DataFrame DataFrame::readCSV(const std::string &filePath, const std::string& separator, bool hasHeaderLine) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file: " + filePath);
    }
    std::vector<std::string> columnNames;
    DataFrame df;

    if(hasHeaderLine) {
        std::string headerLine;
        std::getline(file, headerLine);
        std::stringstream headerStream(headerLine);

        std::string columnName;
        while (std::getline(headerStream, columnName, separator[0])) {
            df.addColumn(columnName);
        }
    }
    else {
        std::string firstLine;
        if (std::getline(file, firstLine)) {
            std::stringstream firstLineStream(firstLine);
            std::string cell;
            size_t inferredColumnCount = 0;

            while (std::getline(firstLineStream, cell, separator[0])) {
                ++inferredColumnCount;
                df.addColumn();
            }

            std::stringstream rowStream(firstLine);
            std::vector<std::optional<ColumnType>> rowValues(df.numberOfColumns());
            size_t colIndex = 0;
            while(std::getline(rowStream, cell, separator[0])) {
                if (colIndex < df.numberOfColumns()) {
                    if(cell.empty()) {
                        rowValues[inferredColumnCount] = std::nullopt;
                    }
                    else {
                        try {
                            double doubleVal = std::stod(cell);
                            rowValues[colIndex] = doubleVal;
                        } catch (...) {
                            try {
                                int intVal = std::stoi(cell);
                                rowValues[colIndex] = intVal;
                            } catch (...) {
                                try {
                                    bool boolVal = (bool)std::stoi(cell);
                                    rowValues[colIndex] = boolVal;
                                }
                                catch (...) {
                                    rowValues[colIndex] = cell;
                                }
                            }
                        }
                    }
                }
                ++colIndex;
            }
            df.addRow(rowValues);
        }
    }

    std::string rowLine;
    while(std::getline(file, rowLine)) {
        std::stringstream rowStream(rowLine);
        std::string cell;
        size_t colIndex = 0;

        std::vector<std::optional<ColumnType>> rowValues(df.numberOfColumns());
        while(std::getline(rowStream, cell, separator[0])) {
            if (colIndex < df.numberOfColumns()) {
                if(cell.empty()) {
                    rowValues[colIndex] = std::nullopt;
                }
                else {
                    try {
                        double doubleVal = std::stod(cell);
                        rowValues[colIndex] = doubleVal;
                    } catch (...) {
                        try {
                            int intVal = std::stoi(cell);
                            rowValues[colIndex] = intVal;
                        } catch (...) {
                            try {
                                bool boolVal = (bool)std::stoi(cell);
                                rowValues[colIndex] = boolVal;
                            }
                            catch (...) {
                                rowValues[colIndex] = cell;
                            }
                        }
                    }
                }
            }
            ++colIndex;
        }
        df.addRow(rowValues);
    }
    return df;
}

void DataFrame::saveCSV(const std::string &filePath, const std::string &separator, bool saveHeaderLine) {
    std::ofstream file(filePath);
    if(!file.is_open()) {
        throw std::runtime_error("Could not open the file: " + filePath);
    }

    if(saveHeaderLine) {
        size_t cnt = 0;
        for(const auto& pair : columns) {
            file << pair.first;
            cnt++;
            if(cnt < columns.size()) {
                file << separator;
            }
        }
        file << "\n";
    }

    for(size_t i = 0; i < this->numberOfRows(); i++) {
        size_t cnt = 0;
        for(const auto& pair : columns) {
            auto column = pair.second;
            if(column[i].has_value()) {
                std::visit([&file](auto&& val) { file << val; }, column[i].value());
            }
            cnt++;
            if(cnt < columns.size()) {
                file << separator;
            }
        }
        file << "\n";
    }
    file.close();
}

void DataFrame::filterColumn(const std::string& columnName, std::function<bool(const ColumnType&)> predicate) {
    if (columns.find(columnName) != columns.end()) {
        Column<std::variant<int, double, bool, std::string>>& column = columns[columnName];

        for (auto& cell : column.getOptionalValues()) {
            if (cell.has_value()) {
                bool shouldKeep = std::visit([&](auto&& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, ColumnType>) {
                        return predicate(value);
                    } else {
                        return true;
                    }
                }, cell.value());

                if (!shouldKeep) {
                    cell = std::nullopt;
                }
            }
        }
    } else {
        std::cerr << "Column " << columnName << " does not exist!" << std::endl;
    }
}

std::vector<std::optional<ColumnType>> DataFrame::getRow(size_t index) const {
    std::vector<std::optional<ColumnType>> row;
    for (const auto& columnPair : this->columns) {
        row.push_back(columnPair.second.getOptionalValues()[index]);
    }
    return row;
}

std::vector<std::optional<ColumnType>> DataFrame::getRowWithoutGroupByColumn(size_t index, const std::string& columnName) const {
    std::vector<std::optional<ColumnType>> row;
    for (const auto& columnPair : this->columns) {
        if(columnPair.first != columnName) {
            row.push_back(columnPair.second.getOptionalValues()[index]);
        }
    }
    return row;
}

DataFrame DataFrame::groupBy(const std::string &columnName, const std::string &aggregation) {
    std::map<ColumnType, std::vector<std::vector<std::optional<ColumnType>>>> groups;
    const Column<ColumnType>& groupColumn = columns.at(columnName);
    size_t numRows = groupColumn.size();

    for (size_t i = 0; i < numRows; ++i) {
        if (groupColumn.getOptionalValues()[i].has_value()) {
            ColumnType groupValue = groupColumn.getOptionalValues()[i].value();
            groups[groupValue].push_back(getRowWithoutGroupByColumn(i, columnName));
        }
    }

    DataFrame result(*this, columnName);

    for(const auto& group : groups) {
        std::vector<std::optional<ColumnType>> aggregatedRow;
            if(aggregation == "sum") {
                std::vector<double> sums = aggregateSum(group.second);
                for (auto sum : sums) {
                    aggregatedRow.push_back(std::make_optional<ColumnType>(sum));
                }
            }
        result.addRow(aggregatedRow);
    }

    return result;
}

std::vector<double> DataFrame::aggregateSum(const std::vector<std::vector<std::optional<ColumnType>>>& groupRows) const {
    std::vector<double> sum(groupRows.size(), 0.0);

    for (const auto& row : groupRows) {
        for (size_t i = 0; i < row.size(); ++i) {
            const auto& cell = row[i];
            if (cell.has_value()) {
                sum[i] += std::visit([](const auto& value) -> double {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                        return static_cast<double>(value);
                    }
                    return 0.0;
                }, cell.value());
            }
        }
    }
    return sum;
}

int main() {
//    Column<std::string> col1("col1");
//    col1.add("AAA");
//    col1.add("BBB");
//    Column<double> col2("col2");
//    col2.add(4.2);
//    col2.add(true);
//    DataFrame df;
//    df.addColumn(col1);
//    df.addColumn(col2);
//
//    df.print();
//    std::vector<std::optional<ColumnType>> newRow = {
//            std::make_optional<std::string>("Charlie"),
//            std::make_optional<double>(35.5),// Age
//             // Name
//    };
//    df.addRow(newRow);
//    df.print();
//    std::vector<std::string> op = {"mean", "std"};
//    auto results = df.aggregate(op);
//    for (const auto& [colName, colResults] : results) {
//        std::cout << "Column: " << colName << "\n";
//        for (const auto& [opName, value] : colResults) {
//            std::cout << "  " << opName << ": ";
//            std::visit([](const auto& val) { std::cout << val << "\n"; }, value);
//        }
//    }
//
//    DataFrame sortedDf = df.sortBy("col2");
//    sortedDf.print();
//
//    std::cout << df.numberOfColumns() << " " << df.numberOfRows();
    DataFrame df = DataFrame::readCSV("../test1.csv", ",", true);
    df.print();
    auto x = df.describe();
    for(const auto& pair : x) {
        std::cout << pair.first << std::endl;
        for(const auto& secondPair : pair.second) {
            std::cout << secondPair.first << ": ";
            std::visit([](const auto& value) {
                std::cout << value;
            }, secondPair.second);
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    df.saveCSV("../test2.csv", ";", true);

    df.groupBy("col1", "sum").print();
}
