#ifndef ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
#define ABSTRACTPROGRAMMINGPROJECT_COLUMN_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>
#include "exceptions.h"

template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept StringType = std::is_same_v<T, std::string>;

template<typename T>
concept Sortable =
        requires(T a, T b) {
            a < b;
            a > b;
            a == b;
        };

template<class DataType>
class Column {
private:
    std::string name;
    std::vector<std::optional<DataType>> values;
    bool isPartOfDataFrame;

public:
    // CONSTRUCTORS
    Column() : name("Unnamed"), isPartOfDataFrame(false) {}
    Column(const std::string& columnName)
            : name(columnName), isPartOfDataFrame(false) {}

    // BASIC HANDLING
    std::string getName() const;
    std::vector<std::optional<DataType>> getOptionalValues() const;
    std::vector<DataType> getValues() const;
    void setName(const std::string& n);
    void setIsPartOfDataFrame(bool p);
    bool getIsPartOfDataFrame() const;
    size_t size() const;
    bool isEmpty() const;
    std::string getDataType() const;
    template<class T> bool isCompatible(const T& value) const;
    bool isNull(size_t index) const;
    void print() const;
    void checkDataFrameIntegrity() const;

    // DATA MANIPULATION
    std::vector<size_t> find(const DataType& element) const;
    void add(const std::optional<DataType>& element);
    void add(const std::optional<DataType>& element, size_t index);
    void removeAt(size_t index);
    void remove(const DataType& element);
    void removeAll(const DataType& element);
    void update(size_t index, const DataType& element);
    template<class Iterable> void addAll(const Iterable& it);
    void removeNull();
    void replace(const DataType& oldValue, const DataType& newValue);
    template<class Predicate> void removeIf(Predicate pred);
    void fillNull(const DataType& value);
    void fillNullWithMean() requires Numeric<DataType>;

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
    DataType mode() const;

    // COUNT BASED AGGREGATIONS
    size_t countNonNull() const;
    size_t countNull() const;
    size_t countDistinct() const;

    // FREQUENCY
    std::map<DataType, size_t> valueCounts() const;
    std::vector<size_t> histogram(size_t numberOfBins) const requires Numeric<DataType>;

    // STRING BASED METHODS
    std::string concatenate(const std::string& linker) const requires StringType<DataType>;

    // SORT
    void sort(bool ascending) requires Sortable<DataType>;

    // FILTER
    template<class Predicate> Column<DataType> filter(Predicate pred) const;

    // OPERATORS
    Column<DataType> applyOperation(const DataType& value, std::function<DataType(const DataType&, const DataType&)> op) const requires Numeric<DataType>;
    Column<DataType> applyOperation(const Column<DataType>& other, std::function<DataType(const DataType&, const DataType&)> op) const requires Numeric<DataType>;
    Column<DataType> operator+(const DataType& value) const requires Numeric<DataType>;
    Column<DataType> operator-(const DataType& value) const requires Numeric<DataType>;
    Column<DataType> operator*(const DataType& value) const requires Numeric<DataType>;
    Column<DataType> operator/(const DataType& value) const requires Numeric<DataType>;
    Column<DataType> operator+(const Column<DataType>& other) const requires Numeric<DataType>;
    Column<DataType> operator-(const Column<DataType>& other) const requires Numeric<DataType>;
    Column<DataType> operator*(const Column<DataType>& other) const requires Numeric<DataType>;
    Column<DataType> operator/(const Column<DataType>& other) const requires Numeric<DataType>;

};

#endif //ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
