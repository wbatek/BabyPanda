#include "../include/column.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <set>

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

    os << "|" << std::setw(maxWidth + 2) << col.getName() << "|" << std::endl;
    std::string separator(maxWidth + 2, '-');
    os << "|" << separator << "|" << std::endl;
    for (const auto& value : col.getOptionalValues()) {
        if (value.has_value()) {
            os << "| " << std::setw(maxWidth) << value.value() << " |" << std::endl;
        } else {
            os << "| " << std::setw(maxWidth) << "-" << " |" << std::endl;
        }
    }
    return os;
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
    this->values.push_back(element);
}

template<class DataType>
void Column<DataType>::add(const std::optional<DataType> &element, size_t index) {
    if (index >= this->values.size()) {
        throw InvalidIndexException();
    }
    this->values.insert(this->values.begin() + index, element);
}

template<class DataType>
void Column<DataType>::removeAt(size_t index) {
    if (index >= this->values.size()) {
        throw InvalidIndexException();
    }
    this->values.erase(this->values.begin() + index);
}

template<class DataType>
void Column<DataType>::remove(const DataType &element) {
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
    if (index >= this->values.size()) {
        throw InvalidIndexException();
    }
}

template<class DataType>
template<class Iterable>
void Column<DataType>::addAll(const Iterable &it) {
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
    for(size_t i = 0; i < this->values->size();) {
        if(!this->values[i].has_value()) {
            this->values.erase(this->values.begin() + i);
        } else {
            i++;
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
    return this->max() - this->min();
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

int main() {
    Column<int> col("name");
    col.add(1);
    col.add(2);
    col.add(20);
    col.add(16);
    col.add(4);
    col.add(11);
    std::cout << col.countNonNull() << std::endl;
    std::cout << col.min() << std::endl;
    std::cout << col.max() << std::endl;
    std::cout << col.mean() << std::endl;
    std::cout << col.median() << std::endl;
}

