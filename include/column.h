#ifndef ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
#define ABSTRACTPROGRAMMINGPROJECT_COLUMN_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>
#include "exceptions.h"
#include <random>

template<typename T>
concept NumericVariant = requires(T v) {
    { std::holds_alternative<int>(v) } -> std::convertible_to<bool>;
    { std::holds_alternative<double>(v) } -> std::convertible_to<bool>;
};

template<typename T>
concept Numeric = std::is_arithmetic_v<T> || NumericVariant<T>;

template<typename T>
concept DecayedOrDirectNumeric = Numeric<T> || Numeric<std::decay_t<T>>;

template<typename T>
concept StringType = std::is_same_v<T, std::string>;

template<typename T>
concept Sortable =
        requires(T a, T b) {
            a < b;
            a > b;
            a == b;
        };

using ColumnType = std::variant<int, double, bool, std::string>;

template<class DataType>
class Column {
private:
    std::string name;
    std::vector<std::optional<DataType>> values;
    bool isPartOfDataFrame;

    std::string generateRandomName(size_t length = 8) {
        const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, chars.size() - 1);

        std::string randomName;
        for (size_t i = 0; i < length; ++i) {
            randomName += chars[dist(gen)];
        }
        return randomName;
    }

public:
    // CONSTRUCTORS
    Column() : name(generateRandomName()) {}
    Column(const std::string& columnName)
            : name(columnName) {}
    Column(const std::string& columnName, const std::vector<std::optional<DataType>>& values)
            : name(columnName), values(values) {}
//    Column(std::string name, std::vector<DataType> values)
//            : name(name), values(values) {}
    Column(const Column& other) = default;
    Column& operator=(const Column& other) = default;
    Column(Column&& other) noexcept = default;
    Column& operator=(Column&& other) noexcept = default;

    // BASIC HANDLING
    std::vector<std::optional<DataType>> getOptionalValues() const;
    std::vector<DataType> getValues() const;
    void setIsPartOfDataFrame(bool p);
    bool getIsPartOfDataFrame() const;
    size_t size() const;
    bool isEmpty() const;
    std::string getDataType() const;
    template<class T> bool isCompatible(const T& value) const;
    bool isNull(size_t index) const;
    void print() const;
    void checkDataFrameIntegrity() const;
    size_t getWidth() const;
    std::string getName() const { return this->name; }

    // DATA MANIPULATION
    std::vector<size_t> find(const DataType& element) const;
    void add(const std::optional<ColumnType>& element);
    void addToColumnFromRow(const std::optional<DataType>& value);
    void add(const std::optional<ColumnType>& element, size_t index);
    void removeAt(size_t index);
    void removeAtFromRow(size_t index);
    void remove(const DataType& element);
    void removeAll(const DataType& element);
    void update(size_t index, const DataType& element);
    template<class Iterable> void addAll(const Iterable& it);
    void removeNull();
    void replace(const DataType& oldValue, const DataType& newValue);
    template<class Predicate> void removeIf(Predicate pred);
    void fillNull(const DataType& value);
    void fillNullWithMean() requires Numeric<DataType>;
    void fillNull();

    // AGGREGATIONS
    DataType min() const;
    DataType max() const;
    double mean() const requires DecayedOrDirectNumeric<DataType>;
    double median() const requires DecayedOrDirectNumeric<DataType>;
    double std() const requires DecayedOrDirectNumeric<DataType>;
    double var() const requires DecayedOrDirectNumeric<DataType>;
    DataType percentile(double p) const requires DecayedOrDirectNumeric<DataType>;
    double skewness() const requires DecayedOrDirectNumeric<DataType>;
    DataType range() const requires DecayedOrDirectNumeric<DataType>;
    DataType mode() const;

    // COUNT BASED AGGREGATIONS
    int countNonNull() const;
    int countNull() const;
    int countDistinct() const;

    // FREQUENCY
    std::map<DataType, size_t> valueCounts() const;
    std::vector<size_t> histogram(size_t numberOfBins) const requires DecayedOrDirectNumeric<DataType>;

    // STRING BASED METHODS
    std::string concatenate(const std::string& linker) const requires StringType<DataType>;

    // SORT
    Column<DataType> sort(bool ascending) requires Sortable<DataType>;

    // FILTER
    template<class Predicate> Column<DataType> filter(Predicate pred) const;

    // OPERATORS
    Column<DataType> applyOperation(const DataType& value, std::function<DataType(const DataType&, const DataType&)> op) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> applyOperation(const Column<DataType>& other, std::function<DataType(const DataType&, const DataType&)> op) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator+(const DataType& value) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator-(const DataType& value) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator*(const DataType& value) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator/(const DataType& value) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator+(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator-(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator*(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType>;
    Column<DataType> operator/(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType>;
    std::optional<DataType> operator[](size_t index) const;
};

#endif //ABSTRACTPROGRAMMINGPROJECT_COLUMN_H
