#include "include/dataframe.h"
#include "include/exceptions.h"
#include "include/column.h"
#include "src/column.cpp"
#include <iostream>
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

//void DataFrame::addRow(const std::vector<std::optional<std::any>>& row) {
//    if(row.size() != numberOfColumns()) {
//        throw InvalidNumberOfColumnsException();
//    }
//
//    for(size_t i = 0; i < columns.size(); i++) {
//        auto& column = columns[i];
//        if(row[i].has_value()) {
//            column->addToColumnFromRow(row[i].value());
//        }
//        else {
//            column->addToColumnFromRow(std::nullopt);
//        }
//    }
//}

//void DataFrame::removeRow(size_t index) {
//    if(index >= numberOfRows()) throw InvalidIndexException();
//    for(auto & column : columns) {
//        column.removeAtFromRow(index);
//    }
//}

//void DataFrame::renameColumn(size_t index, const std::string& newName) {
//    if(index >= numberOfColumns()) throw InvalidIndexException();
//    columns[index]->setName(newName);
//}

//void DataFrame::removeColumn(const std::string& columnName) {
//    auto it = std::find_if(columns.begin(), columns.end(),
//                           [&columnName](const std::shared_ptr<BaseColumn>& column) {
//                               return column->getName() == columnName; // Assuming BaseColumn has a getName() method
//                           });
//
//    if (it != columns.end()) {
//        columns.erase(it);
//    }
//}

//void DataFrame::removeColumn(size_t index) {
//    if(index >= numberOfColumns()) throw InvalidIndexException();
//    columns.erase(columns.begin() + index);
//}

// END DATA MANIPULATION

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

void DataFrame::addRow(const std::vector<std::optional<ColumnType>>& row) {
    if(row.size() != this->numberOfColumns()) {
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
    for(auto& x : getColumns()) {
        x.print();
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
std::map<std::string, double> DataFrame::describe() const {
    std::map<std::string, double> description;
    for(auto& pair : columns) {
        const auto& column = pair.second;
        std::vector<double> statistics;

        description[column.getName() + "_mean"] = column.mean();
        description[column.getName() + "_std"] = column.std();
        description[column.getName() + "_var"] = column.var();
        description[column.getName() + "_max"] = std::get<double>(column.max());
        description[column.getName() + "_min"] = std::get<double>(column.min());
        description[column.getName() + "_median"] = column.median();
        description[column.getName() + "_countNull"] = column.countNull();
        description[column.getName() + "_nunique"] = column.countDistinct();
    }
    return description;
}


int main() {
    Column<std::string> col1("col1");
    col1.add("AAA");
    col1.add("BBB");
    Column<double> col2("col2");
    col2.add(4.2);
    col2.add(true);
    DataFrame df;
    df.addColumn(col1);
    df.addColumn(col2);

    df.print();
    std::vector<std::optional<ColumnType>> newRow = {
            std::make_optional<std::string>("Charlie"),
            std::make_optional<double>(35.5),// Age
             // Name
    };
    df.addRow(newRow);
    df.print();
    auto desc = df.describe();
    for(auto& pair : desc) {
        std::cout << pair.first << " " << pair.second << std::endl;
    }

    std::cout << df.numberOfColumns() << " " << df.numberOfRows();
}
