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
    bool isNullable;
    DataType defaultValue;

public:
    Column(const std::string& columnName, bool nullable = false, const DataType& defaultVal = DataType())
            : name(columnName), isNullable(nullable), defaultValue(defaultVal) {}
    // BASIC HANDLING
    std::string getName();
    std::vector<std::optional<DataType>> getOptionalValues();
    std::vector<DataType> getValues();
    void setName(const std::string& n);
    size_t size() const;
    bool isEmpty() const;
    std::string getDataType() const;
    template<class T> bool isCompatible(const T& value) const;
    bool isNull(size_t index) const;

    // AGGREGATIONS
    DataType min() const requires Numeric<DataType>;
    DataType max() const requires Numeric<DataType>;
    DataType mean() const requires Numeric<DataType>;
    DataType median() const requires Numeric<DataType>;
    double std() const requires Numeric<DataType>;
    double var() const requires Numeric<DataType>;
    DataType percentile(double p) const requires Numeric<DataType>;
    double skewness() const;

    // COUNT BASED AGGREGATIONS
    size_t countNonNull() const;
    size_t countNull() const;
    size_t countDistinct() const;

    // FREQUENCY
    std::map<DataType, size_t> valueCounts() const;
    std::map<DataType, size_t> histogram(size_t bins) const;

    // TO STRING
    std::string toString() const;
};

#endif //ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
