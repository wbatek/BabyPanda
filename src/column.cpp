#include "../include/column.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <set>
#include <functional>

// BASIC HANDLING

template<class DataType>
std::string Column<DataType>::getName() const {
    return this->name;
}

template<class DataType>
std::vector<std::optional<DataType>> Column<DataType>::getOptionalValues() const {
    return this->values;
}

template<class DataType>
std::vector<DataType> Column<DataType>::getValues() const {
    std::vector<DataType> extractedValues;
    for(auto& x : this->values) {
        if(x.has_value()) {
            extractedValues.push_back(x.value());
        }
    }
    return extractedValues;
}

template<class DataType>
void Column<DataType>::setName(const std::string& n) {
    this->name = n;
}

template<class DataType>
void Column<DataType>::setIsPartOfDataFrame(bool p) {
    this->isPartOfDataFrame = p;
}

template<class DataType>
bool Column<DataType>::getIsPartOfDataFrame() const {
    return this->isPartOfDataFrame;
}

template<class DataType>
size_t Column<DataType>::size() const {
    return this->values.size();
}

template<class DataType>
bool Column<DataType>::isEmpty() const {
    return this->values.size() == 0;
}

template<class DataType>
std::string Column<DataType>::getDataType() const {
    return typeid(DataType).name();
}

// TODO
template<class DataType>
template<class T>
bool Column<DataType>::isCompatible(const T& value) const {
    if constexpr (std::is_constructible_v<DataType, T>) {
        try {
            DataType temp = static_cast<DataType>(value);
            (void)temp;
            return true;
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }
}

template<class DataType>
bool Column<DataType>::isNull(size_t index) const {
    if(index > this->size()) {
        throw InvalidIndexException();
    }
    return !this->values[index].has_value();
}

template<class DataType>
void Column<DataType>::print() const {
    std::cout << *this;
}

template<class DataType>
std::ostream& operator<<(std::ostream& os, const Column<DataType>& col) {
    size_t maxWidth = col.getName().length();
    for(const auto& val : col.getOptionalValues()) {
        if(val.has_value()) {
            size_t currentWidth = std::to_string(val.value()).length();
            maxWidth = std::max(maxWidth, currentWidth);
        }
        else {
            maxWidth = std::max(maxWidth, size_t(1));
        }
    }

    os << "|" << std::setw(maxWidth) << col.getName() << "|" << std::endl;
    std::string separator(maxWidth, '-');
    os << "|" << separator << "|" << std::endl;
    for (const auto& value : col.getOptionalValues()) {
        if (value.has_value()) {
            os << "|" << std::setw(maxWidth) << value.value() << "|" << std::endl;
        } else {
            os << "|" << std::setw(maxWidth) << "-" << "|" << std::endl;
        }
    }
    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const Column<std::string>& col) {
    size_t maxWidth = col.getName().length();
    for(const auto& val : col.getOptionalValues()) {
        if(val.has_value()) {
            size_t currentWidth = val.value().length();
            maxWidth = std::max(maxWidth, currentWidth);
        }
        else {
            maxWidth = std::max(maxWidth, size_t(1));
        }
    }

    os << "|" << std::setw(maxWidth) << col.getName() << "|" << std::endl;
    std::string separator(maxWidth, '-');
    os << "|" << separator << "|" << std::endl;
    for (const auto& value : col.getOptionalValues()) {
        if (value.has_value()) {
            os << "|" << std::setw(maxWidth) << value.value() << "|" << std::endl;
        } else {
            os << "|" << std::setw(maxWidth) << "-" << "|" << std::endl;
        }
    }
    return os;
}

template<class DataType>
void Column<DataType>::checkDataFrameIntegrity() const {
    if(isPartOfDataFrame) {
        throw DataFrameIntegrityException();
    }
}

// END BASIC HANDLING

// DATA MANIPULATION

template<class DataType>
std::vector<size_t> Column<DataType>::find(const DataType& element) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i].has_value() && values[i].value() == element) {
            indices.push_back(i);
        }
    }
    return indices;
}


template<class DataType>
void Column<DataType>::add(const std::optional<DataType> &element) {
    checkDataFrameIntegrity();
    this->values.push_back(element);
}

template<class DataType>
void Column<DataType>::add(const std::optional<DataType> &element, size_t index) {
    if (index >= this->values.size()) throw InvalidIndexException();
    checkDataFrameIntegrity();
    this->values.insert(this->values.begin() + index, element);
}

template<class DataType>
void Column<DataType>::removeAt(size_t index) {
    if (index >= this->values.size()) throw InvalidIndexException();
    checkDataFrameIntegrity();
    this->values.erase(this->values.begin() + index);
}

template<class DataType>
void Column<DataType>::remove(const DataType &element) {
    checkDataFrameIntegrity();
    for(size_t i = 0; i < this->values.size(); i++) {
        if(this->values[i].has_value()) {
            if(this->values[i].value() == element) {
                this->values.erase(this->values.begin() + i);
                break;
            }
        }
    }
}

template<class DataType>
void Column<DataType>::removeAll(const DataType& element) {
    checkDataFrameIntegrity();
    for(size_t i = 0; i < this->values.size(); i++) {
        if(this->values[i].has_value()) {
            if(this->values[i].value() == element) {
                this->values.erase(this->values.begin() + i);
            }
        }
    }
}

template<class DataType>
void Column<DataType>::update(size_t index, const DataType& element) {
    if (index >= this->values.size()) throw InvalidIndexException();
    this->values[index] = element;
}

template<class DataType>
template<class Iterable>
void Column<DataType>::addAll(const Iterable &it) {
    checkDataFrameIntegrity();
    for(const auto& elem : it) {
        if (isCompatible(elem)) {
            this->add(std::optional<DataType>(elem));
        } else {
            throw InvalidTypeException();
        }
    }
}

template<class DataType>
void Column<DataType>::removeNull() {
    checkDataFrameIntegrity();
    for(size_t i = 0; i < this->values->size();) {
        if(!this->values[i].has_value()) {
            this->values.erase(this->values.begin() + i);
        } else {
            i++;
        }
    }
}

template<class DataType>
void Column<DataType>::replace(const DataType &oldValue, const DataType &newValue) {
    for (auto& val : values) {
        if(val.has_value() && val.value() == oldValue) {
            val = newValue;
        }
    }
}

template<class DataType>
template<class Predicate>
void Column<DataType>::removeIf(Predicate pred) {
    checkDataFrameIntegrity();
    auto it = std::remove_if(values.begin(), values.end(),
                             [&](const std::optional<DataType>& val) {
                                 return val.has_value() && pred(val.value());
                             });
    values.erase(it, values.end());
}

template<class DataType>
void Column<DataType>::fillNull(const DataType &value) {
    for(size_t i = 0; i < this->values.size(); i++) {
        if(!this->values[i].has_value()) {
            this->values[i] = value;
        }
    }
}

template<class DataType>
void Column<DataType>::fillNullWithMean() requires Numeric<DataType> {
    DataType mean = this->mean();
    for(size_t i = 0; i < this->values.size(); i++) {
        if(!this->values[i].has_value()) {
            this->values[i] = mean;
        }
    }
}

// END DATA MANIPULATION

// AGGREGATIONS

template<class DataType>
DataType Column<DataType>::min() const requires Numeric<DataType> {
    if(this->isEmpty()) {
        throw EmptyColumnException();
    }
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    DataType minValue;
    bool foundValidValue = false;
    std::vector<DataType> filtederValues = this->getValues();
    for(const auto& val : filtederValues) {
        if (!foundValidValue || val < minValue) {
            minValue = val;
            foundValidValue = true;
        }
    }
    return minValue;
}

template<class DataType>
DataType Column<DataType>::max() const requires Numeric<DataType> {
    if(this->isEmpty()) {
        throw EmptyColumnException();
    }
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    DataType maxValue;
    bool foundValidValue = false;
    std::vector<DataType> filteredValues = this->getValues();
    for(const auto& val : filteredValues) {
        if (!foundValidValue || val > maxValue) {
            maxValue = val;
            foundValidValue = true;
        }
    }
    return maxValue;
}

template<class DataType>
double Column<DataType>::mean() const requires Numeric<DataType> {
    if(this->isEmpty()) {
        throw EmptyColumnException();
    }
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    DataType sum = DataType();
    for(const auto& val : this->getValues()) {
        sum += val;
    }
    return sum / this->getValues().size();
}

template<class DataType>
double Column<DataType>::median() const requires Numeric<DataType> {
    if(this->isEmpty()) throw EmptyColumnException();
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    std::vector<DataType> filteredValues = this->getValues();
    std::sort(filteredValues.begin(), filteredValues.end());
    size_t size = filteredValues.size();
    if(filteredValues.size() % 2 == 0) {
        return static_cast<double>((filteredValues[size / 2 - 1] + filteredValues[size / 2]) / 2.0);
    }
    return static_cast<double>(filteredValues[size / 2]);
}

template<class DataType>
double Column<DataType>::std() const requires Numeric<DataType> {
    if(this->isEmpty()) throw EmptyColumnException();
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    double mean = this->mean();
    double ssq = 0.0;
    for(const auto& val : this->getValues()) {
        ssq += (val - mean) * (val - mean);
    }
    return std::sqrt(ssq / (this->getValues().size() - 1));
}

template<class DataType>
double Column<DataType>::var() const requires Numeric<DataType> {
    return std::pow(this->std(), 2);
}

template<class DataType>
DataType Column<DataType>::percentile(double p) const requires Numeric<DataType> {
    if(this->isEmpty()) throw EmptyColumnException();
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    if (p < 0.0 || p > 1.0) {
        throw std::invalid_argument("Percentile must be between 0 and 1.");
    }
    std::vector<DataType> filteredValues = this->getValues();
    std::sort(filteredValues.begin(), filteredValues.end());
    size_t n = filteredValues.size();

    double pos = (n - 1) * p;
    auto lower_index = static_cast<size_t>(std::floor(pos));
    auto upper_index = static_cast<size_t>(std::ceil(pos));

    if (lower_index == upper_index) {
        return filteredValues[lower_index];
    } else {
        double lower_value = filteredValues[lower_index];
        double upper_value = filteredValues[upper_index];
        return lower_value + (pos - lower_index) * (upper_value - lower_value);
    }
}

template<class DataType>
double Column<DataType>::skewness() const requires Numeric<DataType> {
    double mean = this->mean();
    double std = this->std();
    double n = this->getValues().size();
    std::vector<DataType> filteredValues = this->getValues();

    if (n < 3) {
        throw std::invalid_argument("Skewness requires at least 3 data points.");
    }

    double sum_cubic_diff = 0.0;
    for (int val : filteredValues) {
        double deviation = val - mean;
        sum_cubic_diff += std::pow(deviation / std, 3);
    }
    return (n / ((n - 1) * (n - 2))) * sum_cubic_diff;
}

template<class DataType>
DataType Column<DataType>::range() const requires Numeric<DataType> {
    return this->max() - this->min() + 1;
}

template<class DataType>
DataType Column<DataType>::mode() const {
    if(isEmpty()) throw EmptyColumnException();
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }

    std::map<DataType, size_t> valueCounts = this->valueCounts();
    auto modeIt = std::max_element(valueCounts.begin(), valueCounts.end(),
                                   [](const auto& a, const auto& b) {
                                        return a.second < b.second;
    });
    return modeIt->first;
}

// END AGGREGATIONS

// COUNT BASED AGGREGATIONS

template<class DataType>
size_t Column<DataType>::countNonNull() const {
    return this->getValues().size();
}

template<class DataType>
size_t Column<DataType>::countNull() const {
    return this->getOptionalValues().size() - this->getValues().size();
}

template<class DataType>
size_t Column<DataType>::countDistinct() const {
    std::set<DataType> s(this->getValues().begin(), this->getValues().end());
    return s.size();
}

// END COUNT BASED AGGREGATIONS

// FREQUENCY

template<class DataType>
std::map<DataType, size_t> Column<DataType>::valueCounts() const {
    std::map<DataType, size_t> map;
    for(auto& val : this->getValues()) {
        map[val]++;
    }
    return map;
}

template<class DataType>
std::vector<size_t> Column<DataType>::histogram(size_t numberOfBins) const requires Numeric<DataType> {
    DataType range = this->range();
    DataType min = this->min();
    std::vector<size_t> bins(numberOfBins, 0);
    for(auto& x : this->getValues()) {
        int bin = std::floor((x - min) * numberOfBins / range);
        bins[bin]++;
    }
    return bins;
}


// END FREQUENCY

// STRING BASED METHODS

template<class DataType>
std::string Column<DataType>::concatenate(const std::string& linker) const requires StringType<DataType> {
    std::string result;
    std::vector<DataType> filteredValues = this->getValues();
    for(size_t i = 0; i < filteredValues.size(); i++) {
        result += filteredValues[i] + linker;
    }
    result += filteredValues[filteredValues.size() - 1];
    return result;
}

// END STRING BASED METHODS

// SORT

template<class DataType>
void Column<DataType>::sort(bool ascending) requires Sortable<DataType> {
    checkDataFrameIntegrity();
    if(ascending) {
        std::sort(this->values.begin(), this->values.end());
    }
    else {
        std::sort(this->values.begin(), this->values.end(), std::greater<>());
    }
}

// FILTER

template<class DataType>
template<class Predicate>
Column<DataType> Column<DataType>::filter(Predicate pred) const {
    Column<DataType> result(this->name + "_filtered");
    for(const auto& val : this->getValues()) {
        if(pred(val)) {
            result.add(val);
        }
    }
    return result;
}

// OPERATORS

// helper method

template<class DataType>
Column<DataType> Column<DataType>::applyOperation(const DataType& value, std::function<DataType(const DataType&, const DataType&)> op) const requires Numeric<DataType> {
    Column<DataType> result(this->name + "_operation");
    for(const auto& val : this->values) {
        if(val.has_value()) {
            result.add(op(val.value(), value));
        }
        else {
            result.add(std::nullopt);
        }
    }
    return result;
}

template<class DataType>
Column<DataType> Column<DataType>::operator+(const DataType& value) const requires Numeric<DataType> {
    return applyOperation(value, std::plus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator-(const DataType& value) const requires Numeric<DataType> {
    return applyOperation(value, std::minus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator*(const DataType& value) const requires Numeric<DataType> {
    return applyOperation(value, std::multiplies<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator/(const DataType& value) const requires Numeric<DataType> {
    return applyOperation(value, std::divides<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::applyOperation(const Column<DataType>& other, std::function<DataType(const DataType&, const DataType&)> op) const requires Numeric<DataType> {
    if (this->values.size() != other.values.size()) {
        throw InvalidSizeException();
    }

    Column<DataType> result(this->name + "_operation_" + other.name);

    for (size_t i = 0; i < this->values.size(); ++i) {
        if (this->values[i].has_value() && other.values[i].has_value()) {
            result.add(op(this->values[i].value(), other.values[i].value()));
        } else {
            result.add(std::nullopt);
        }
    }

    return result;
}

template<class DataType>
Column<DataType> Column<DataType>::operator+(const Column<DataType>& other) const requires Numeric<DataType> {
    return applyOperation(other, std::plus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator-(const Column<DataType>& other) const requires Numeric<DataType> {
    return applyOperation(other, std::minus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator*(const Column<DataType>& other) const requires Numeric<DataType> {
    return applyOperation(other, std::multiplies<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator/(const Column<DataType>& other) const requires Numeric<DataType> {
    return applyOperation(other, std::divides<DataType>());
}




int main() {
//    Column<int> col("n");
//    col.add(1);
//    col.add(2);
//    col.add(20);
//    col.add(16);
//    col.add(std::nullopt);
//    col.add(4);
//    col.add(11);
//    auto evenFilter = [](int value) { return value % 2 == 0; };
//    Column<int> filteredCol = col.filter(evenFilter);
//    filteredCol.print();
//    col.print();
//    Column<int> col1("Column1");
//    col1.add(1);
//    col1.add(2);
//    col1.add(4);
//
//    Column<int> col2("Column2");
//    col2.add(5);
//    col2.add(std::nullopt); // Add a null value
//    col2.add(3);
//    col2.add(2);
//
//    Column<int> resultColAdd = col1 + col2;
//    Column<int> resultColSub = col1 - col2;
//    Column<int> resultColMul = col1 * col2;
//    Column<int> resultColDiv = col1 / col2; // Ensure no division by zero
//
//    resultColAdd.print(); // Should print 6, null, null, 6
//    resultColSub.print(); // Should print -4, null, null, 2
//    resultColMul.print(); // Should print 5, null, null, 8
//    resultColDiv.print(); // Should print 0.2, null, null, 2

    Column<int> colInt("IntColumn");
    colInt.add(1);
    colInt.add(2);
    colInt.add(2);
    colInt.add(3);
    std::cout << colInt.mode();
}

