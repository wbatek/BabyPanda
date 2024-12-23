#include "../include/column.h"
#include <iostream>

// BASIC HANDLING

template<class DataType>
std::string Column<DataType>::getName() {
    return this->name;
}

template<class DataType>
std::vector<std::optional<DataType>> Column<DataType>::getOptionalValues() {
    return this->values;
}

template<class DataType>
std::vector<DataType> Column<DataType>::getValues() {
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

template<class DataType>
template<class T>
bool Column<DataType>::isCompatible(const T& value) const {
    if constexpr (std::is_constructible_v<DataType, T>) {
        try {
            DataType temp = static_cast<DataType>(value);
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

// END BASIC HANDLING

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
    for (size_t i = 0; i < this->values.size(); ++i) {
        if (this->values[i].has_value()) {
            if (!foundValidValue || this->values[i].value() < minValue) {
                minValue = this->values[i].value();
                foundValidValue = true;
            }
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
    for (size_t i = 0; i < this->values.size(); ++i) {
        if (this->values[i].has_value()) {
            if (!foundValidValue || this->values[i].value() > max()) {
                maxValue = this->values[i].value();
                foundValidValue = true;
            }
        }
    }
    return maxValue;
}

template<class DataType>
DataType Column<DataType>::mean() const requires Numeric<DataType> {
    if(this->isEmpty()) {
        throw EmptyColumnException();
    }
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    DataType sum = DataType();
    for(const DataType& value : values) {
        sum += value;
    }
    return sum / static_cast<DataType>(values.size());
}


