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

void DataFrame::addRow(const std::map<std::string, std::optional<ColumnType>>& row) {
    if (row.size() != this->numberOfColumns()) {
        throw std::invalid_argument("Row size does not match the number of columns");
    }

    for (auto& colPair : columns) {
        const std::string& columnName = colPair.first;
        Column<ColumnType>& column = colPair.second;

        auto it = row.find(columnName);
        if (it != row.end()) {
            const auto& cellValue = it->second;

            if (cellValue.has_value()) {
                bool typeMismatch = false;

                if (column.getValues().empty()) {
                    typeMismatch = false;
                } else {
                    auto firstNonNull = column.getValues()[0];

                    std::visit([&](auto&& value) {
                        using T = std::decay_t<decltype(value)>;
                        try {
                            (void)std::get<T>(*cellValue);
                            typeMismatch = false;
                        } catch (const std::bad_variant_access&) {
                            typeMismatch = true;
                        }
                    }, firstNonNull);
                }

                if (typeMismatch) {
                    if (std::holds_alternative<int>(*cellValue) && column.getValues()[0].index() == 0) {
                        typeMismatch = false;
                    }
                }

                if (typeMismatch) {
                    column.add(std::nullopt);
                } else {
                    column.add(cellValue);
                }
            } else {
                column.add(std::nullopt);
            }
        } else {
            throw std::invalid_argument("Row contains undefined column name: " + columnName);
        }
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
    size_t numRows = columns.begin()->second.size();
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
        std::cout << std::setw(maxWidthFinal) << std::left << columnName << " | ";
    }
    std::cout << std::endl;

    for (const auto& colPair : columns) {
        (void)colPair;
        std::cout << std::setw(maxWidthFinal) << std::left << std::string(columns.size(), '-') << " | ";
    }
    std::cout << std::endl;

    for (size_t i = 0; i < numRows; ++i) {
        for (const auto& colPair : columns) {
            (void)colPair;
            const std::string& columnName = colPair.first;
            (void)columnName;
            const Column<ColumnType>& column = colPair.second;
            const auto& values = column.getOptionalValues();
            const auto& val = values[i];

            if (val.has_value()) {
                std::visit([&](auto&& v) {
                    std::cout << std::setw(maxWidthFinal) << std::left << v << " | ";
                }, val.value());
            } else {
                std::cout << std::setw(maxWidthFinal) << std::left << "null" << " | ";
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

template<typename Predicate>
DataFrame DataFrame::filterRows(const Predicate& pred) const {
    DataFrame filteredDf;

    for (const auto& colPair : this->columns) {
        const std::string& columnName = colPair.first;
        filteredDf.addColumn(columnName);
    }

    size_t numberOfRows = this->numberOfRows();
    for (size_t rowIdx = 0; rowIdx < numberOfRows; ++rowIdx) {
        std::map<std::string, std::optional<ColumnType>> rowMapping;

        for (const auto& colPair : this->columns) {
            const std::string& columnName = colPair.first;
            const Column<ColumnType>& col = colPair.second;

            if (rowIdx < col.size()) {
                rowMapping[columnName] = col[rowIdx];
            } else {
                rowMapping[columnName] = std::nullopt;
            }
        }
        if (pred(rowMapping)) {
            filteredDf.addRow(rowMapping);
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

    DataFrame df;
    std::vector<std::string> columnNames;

    if (hasHeaderLine) {
        std::string headerLine;
        std::getline(file, headerLine);
        std::stringstream headerStream(headerLine);

        std::string columnName;
        while (std::getline(headerStream, columnName, separator[0])) {
            columnNames.push_back(columnName);
            df.addColumn(columnName);
        }
    } else {
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
    while (std::getline(file, rowLine)) {
        std::stringstream rowStream(rowLine);
        std::string cell;
        std::map<std::string, std::optional<ColumnType>> rowValues;

        size_t colIndex = 0;
        while (std::getline(rowStream, cell, separator[0])) {
            if (colIndex < columnNames.size()) {
                const std::string& columnName = columnNames[colIndex];

                if (cell.empty()) {
                    rowValues[columnName] = std::nullopt;
                } else {
                    try {
                        double doubleVal = std::stod(cell);
                        rowValues[columnName] = doubleVal;
                    } catch (...) {
                        try {
                            int intVal = std::stoi(cell);
                            rowValues[columnName] = intVal;
                        } catch (...) {
                            try {
                                bool boolVal = (bool)std::stoi(cell);
                                rowValues[columnName] = boolVal;
                            } catch (...) {
                                rowValues[columnName] = cell;
                            }
                        }
                    }
                }
                ++colIndex;
            }
        }

        // Add the row to the DataFrame
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
            if(aggregation == "count") {
                std::vector<double> counts = aggregateCount(group.second);
                for (auto count : counts) {
                    aggregatedRow.push_back(std::make_optional<ColumnType>(count));
                }
            }
            if(aggregation == "mean") {
                std::vector<double> means = aggregateMean(group.second);
                for (auto mean : means) {
                    aggregatedRow.push_back(std::make_optional<ColumnType>(mean));
                }
            }
        result.addRow(aggregatedRow);
    }

    return result;
}

std::vector<double> DataFrame::aggregateSum(const std::vector<std::vector<std::optional<ColumnType>>>& groupRows) const {
    size_t numColumns = groupRows[0].size();
    std::vector<double> sum(numColumns, 0.0);

    for (const auto& row : groupRows) {
        for (size_t i = 0; i < row.size(); ++i) {
            const auto& cell = row[i];
            if (cell.has_value()) {
                sum[i] += std::visit([](const auto& value) -> double {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                        return static_cast<double>(value);
                    }
                    return std::nan("");
                }, cell.value());
            }
        }
    }
    return sum;
}

std::vector<double> DataFrame::aggregateCount(const std::vector<std::vector<std::optional<ColumnType>>>& groupRows) const {
    size_t numColumns = groupRows[0].size();
    std::vector<double> counts(numColumns, 0.0);

    for (size_t rowIndex = 0; rowIndex < groupRows.size(); ++rowIndex) {
        const auto& row = groupRows[rowIndex];
        for (const auto& cell : row) {
            if (cell.has_value()) {
                counts[rowIndex] += 1.0;
            }
        }
    }
    return counts;
}

std::vector<double> DataFrame::aggregateMean(const std::vector<std::vector<std::optional<ColumnType>>>& groupRows) const {
    if (groupRows.empty() || groupRows[0].empty()) {
        return {};
    }

    size_t numColumns = groupRows[0].size();
    std::vector<double> means(numColumns, 0.0);
    std::vector<size_t> counts(numColumns, 0);

    for (const auto& row : groupRows) {
        for (size_t colIndex = 0; colIndex < numColumns; ++colIndex) {
            const auto& cell = row[colIndex];
            if (cell.has_value()) {
                means[colIndex] += std::visit([](const auto& value) -> double {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                        return static_cast<double>(value);
                    }
                    return std::nan("");
                }, cell.value());
                ++counts[colIndex];
            }
        }
    }

    for (size_t colIndex = 0; colIndex < numColumns; ++colIndex) {
        if (counts[colIndex] > 0) {
            means[colIndex] /= static_cast<double>(counts[colIndex]);
        } else {
            means[colIndex] = 0.0;
        }
    }

    return means;
}

int main() {
    Column<int> intColumn("Age", {25, 30, 35, 40, 45, 50, 55, 10, 33, 17, 30, 30, 30});

    std::cout << "Column: " << intColumn.getName() << std::endl;
    intColumn.print();

    std::cout << "\nColumn Metadata:" << std::endl;
    std::cout << "Name: " << intColumn.getName() << std::endl;
    std::cout << "Size: " << intColumn.size() << std::endl;
    std::cout << "Data Type: " << intColumn.getDataType() << std::endl;
    std::cout << "Is Empty: " << (intColumn.isEmpty() ? "Yes" : "No") << std::endl;

    intColumn.add(60);
    std::cout << "\nAfter adding value 60:" << std::endl;
    intColumn.print();

    intColumn.remove(35);
    std::cout << "\nAfter removing value 35:" << std::endl;
    intColumn.print();

    std::cout << "\nFrequency of Values in Integer Column:" << std::endl;
    auto frequencies = intColumn.valueCounts();
    for (const auto &[value, count]: frequencies) {
        std::cout << value << ": " << count << " times" << std::endl;
    }

    std::cout << "\nMinimum value: " << intColumn.min() << std::endl;
    std::cout << "Maximum value: " << intColumn.max() << std::endl;
    std::cout << "Mean value: " << intColumn.mean() << std::endl;
    std::cout << "Median: " << intColumn.median() << std::endl;

    auto multipliedColumn = intColumn * 2;
    std::cout << "\nColumn after multiplying by 2:" << std::endl;
    multipliedColumn.print();

    auto dividedColumn = intColumn / 5;
    std::cout << "\nColumn after dividing by 5:" << std::endl;
    dividedColumn.print();

    auto filteredColumn = intColumn.filter([](int value) {
        return value >= 30;
    });
    std::cout << "\nColumn after filtering (values >= 30):" << std::endl;
    filteredColumn.print();

    Column<std::string> stringColumn("Names", {"Alice", "Bob", "Charlie", "Diana", "Eve"});
    std::cout << "\nString Column: " << stringColumn.getName() << std::endl;
    stringColumn.print();

    std::string concatenated = stringColumn.concatenate(", ");
    std::cout << "\nConcatenated strings: " << concatenated << std::endl;

    Column<int> nullableColumn("NullableColumn", {10, std::nullopt, 30, 40, std::nullopt});
    std::cout << "\nColumn with nulls: " << nullableColumn.getName() << std::endl;
    nullableColumn.print();

    std::cout << "\nCounts in Nullable Column:" << std::endl;
    std::cout << "Non-Null Count: " << nullableColumn.countNonNull() << std::endl;
    std::cout << "Null Count: " << nullableColumn.countNull() << std::endl;
    std::cout << "Distinct Count: " << nullableColumn.countDistinct() << std::endl;

    nullableColumn.fillNull(99);
    std::cout << "\nColumn after filling nulls with value 99:" << std::endl;
    nullableColumn.print();

    std::cout << "\nColumn after filling nulls with value 99 and sorting:" << std::endl;
    nullableColumn.sort(true).print();

    nullableColumn.replace(99, 100);
    std::cout << "\nNullable Column after replacing 99 with 100:" << std::endl;
    nullableColumn.print();

    auto transformedColumn = intColumn.filter([](int value) { return value > 30; })
            .sort(false)
            .applyOperation(10, [](int a, int b) { return a + b; });

    std::cout << "\nChained Operations Result:" << std::endl;
    transformedColumn.print();

    try {
        intColumn.removeAt(100);
    } catch (const std::exception &e) {
        std::cout << "\nError Handling Demonstration:" << std::endl;
        std::cout << "Caught Exception: " << e.what() << std::endl;
    }

    std::cout << "\n";

    DataFrame df = DataFrame::readCSV("../test1.csv");
    df.print();

    std::cout << "\nDataFrame Metadata:" << std::endl;
    std::cout << "Name: " << df.getName() << std::endl;
    std::cout << "Number of Columns: " << df.numberOfColumns() << std::endl;
    std::cout << "Number of Rows: " << df.numberOfRows() << std::endl;
    std::cout << "Shape: (" << df.shape().first << ", " << df.shape().second << ")" << std::endl;
    std::cout << "Is Empty: " << (df.isEmpty() ? "Yes" : "No") << std::endl;

    std::cout << "\nColumn Names:" << std::endl;
    for (const auto& name : df.columnNames()) {
        std::cout << name << std::endl;
    }

    std::cout << "\nDescribe DataFrame:" << std::endl;
    auto description = df.describe();
    for (const auto& [columnName, stats] : description) {
        std::cout << "Column: " << columnName << std::endl;
        for (const auto& [statName, value] : stats) {
            std::cout << "  " << statName << ": ";
            std::visit([](auto&& arg) {
                std::cout << arg;
            }, value);
            std::cout << std::endl;
        }
    }

    std::cout << "\nSort DataFrame by 'int' column (ascending):" << std::endl;
    auto sortedDf = df.sortBy("int", true);
    sortedDf.print();

    std::cout << "\nFilter Rows where 'double' column > 15.0:" << std::endl;
    auto filteredDf = df.filterRows([&](const std::map<std::string, std::optional<ColumnType>>& rowMapping) {
        if (rowMapping.at("double").has_value()) {
            const auto& cell = rowMapping.at("double");
            if (std::holds_alternative<double>(*cell)) {
                return std::get<double>(*cell) > 15.0;
            }
        }
        return false;
    });
    filteredDf.print();



    std::cout << "\nAdd a Row to the DataFrame:" << std::endl;
    std::map<std::string, std::optional<ColumnType>> rowMapping = {
            {"str", std::string("New")},
            {"int", std::nullopt},
            {"double", 99.99}
    };
    df.addRow(rowMapping);
    df.print();

    std::cout << "\nAdd a Column with invalid size to the DataFrame:" << std::endl;
    try {
        df.addColumn(intColumn);
    } catch (const std::exception &e) {
        std::cout << "\nError Handling Demonstration:" << std::endl;
        std::cout << "Caught Exception: " << e.what() << std::endl;
    }

    std::cout << "\nAdd a Column to the DataFrame:" << std::endl;
    Column<int> columnToAdd("new_col", {25, 30, 35, 40, 45, 50, 55, 10, 33, 17, 30, 1050012});
    df.addColumn(columnToAdd);
    df.print();

    std::cout << "\nGroup by 'str' column and calculate sum over groups:" << std::endl;
    auto groupedDf = df.groupBy("str", "sum");
    groupedDf.print();

    std::cout << "\nGroup by 'str' column and calculate mean over groups:" << std::endl;
    auto groupedDf2 = df.groupBy("str", "mean");
    groupedDf2.print();

    std::cout << "\nGroup by 'int' column and calculate sum over groups:" << std::endl;
    auto groupedDf3 = df.groupBy("int", "sum");
    groupedDf3.print();

    std::cout << "\nSave the DataFrame to 'output.csv':" << std::endl;
    groupedDf.saveCSV("output.csv");

    return 0;
}
