#ifndef ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
#define ABSTRACTPROGRAMMINGPROJECT_COLUMN_H

#include <iostream>
#include <vector>
#include <map>
#include "exceptions.h"

template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept StringType = std::is_same_v<T, std::string>;

template<class DataType>
class Column {
private:
    std::string name;
    std::vector<std::optional<DataType>> values;

public:
    Column(const std::string& columnName)
            : name(columnName) {}
    // BASIC HANDLING
    std::string getName() const;
    std::vector<std::optional<DataType>> getOptionalValues() const;
    std::vector<DataType> getValues() const;
    void setName(const std::string& n);
    size_t size() const;
    bool isEmpty() const;
    std::string getDataType() const;
    template<class T> bool isCompatible(const T& value) const;
    bool isNull(size_t index) const;
    void print() const;

    // DATA MANIPULATION
    void add(const std::optional<DataType>& element);
    void removeAt(size_t index);
    void remove(const DataType& element);
    void removeAll(const DataType& element);
    void update(size_t index, const DataType& element);
    template<class Iterable> void addAll(const Iterable& it);

    // AGGREGATIONS
    DataType min() const requires Numeric<DataType>;
    DataType max() const requires Numeric<DataType>;
    double mean() const requires Numeric<DataType>;
    double median() const requires Numeric<DataType>;
    double std() const requires Numeric<DataType>;
    double var() const requires Numeric<DataType>;
    DataType percentile(double p) const requires Numeric<DataType>;
    double skewness() const requires Numeric<DataType>;
    DataType range() const requires Numeric<DataType>;

    // COUNT BASED AGGREGATIONS
    size_t countNonNull() const;
    size_t countNull() const;
    size_t countDistinct() const;

    // FREQUENCY
    std::map<DataType, size_t> valueCounts() const;
    std::map<DataType, size_t> histogram(size_t bins) const;
};

#endif //ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
