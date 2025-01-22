#include "../include/column.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <set>
#include <functional>
#include <variant>

using ColumnType = std::variant<int, double, bool, std::string>;

// BASIC HANDLING

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

template <typename DataType>
std::ostream& operator<<(std::ostream& os, const Column<DataType>& col) {
    os << "Column: " << col.getName() << "\n";

    for (const auto& optVar : col.getOptionalValues()) {
        if (optVar.has_value()) {
            os << optVar.value();
        } else {
            os << "null";
        }
        os << " ";
    }
    os << "\n";

    return os;
}


template <>
std::ostream& operator<<(std::ostream& os, const Column<ColumnType>& col) {
    // Print the column name
    os << "Column: " << col.getName() << "\n";

    // Print all values in the column
    for (const auto& optVar : col.getOptionalValues()) {
        if (optVar.has_value()) {
            // Use std::visit to handle the variant's value
            std::visit([&os](const auto& value) {
                os << value; // Print the value, deduced type
            }, optVar.value());
        } else {
            os << "null"; // Optional is empty
        }
        os << " ";
    }
    os << "\n";

    return os;
}

template<class DataType>
void Column<DataType>::checkDataFrameIntegrity() const {
    if(isPartOfDataFrame) {
        throw DataFrameIntegrityException();
    }
}

template<class DataType>
size_t Column<DataType>::getWidth() const {
    size_t maxWidth = name.length();

    for (const auto& value : values) {
        size_t currentWidth = 0;
        if (value.has_value()) {
            if constexpr (std::is_same_v<DataType, std::string>) {
                currentWidth = value.value().length();
            } else {
                currentWidth = std::to_string(value.value()).length();
            }
        } else {
            currentWidth = 4;
        }
        maxWidth = std::max(maxWidth, currentWidth);
    }
    return maxWidth;
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
void Column<DataType>::add(const std::optional<ColumnType> &element) {
//    if (!isCompatibleType(element)) {
//        throw std::invalid_argument("Type mismatch in the value added.");
//    }
    checkDataFrameIntegrity();
    if(element.has_value()) {
        values.push_back(std::visit([](auto&& value) -> DataType {
            if constexpr (std::is_convertible_v<std::decay_t<decltype(value)>, DataType>) {
                return static_cast<DataType>(value);
            } else {
                throw std::invalid_argument("Incompatible type in variant.");
            }
        }, *element));
    }
    else {
        values.push_back(std::nullopt);
    }
}

template<class DataType>
void Column<DataType>::addToColumnFromRow(const std::optional<DataType>& value) {
    if(value.has_value()) {
        values.push_back(value.value());
    }
    else {
        values.push_back(std::nullopt);
    }
}

template<class DataType>
void Column<DataType>::add(const std::optional<ColumnType> &element, size_t index) {
    if (index >= this->values.size()) throw InvalidIndexException();
//    if (!isCompatibleType(element)) {
//        throw std::invalid_argument("Type mismatch in the value added.");
//    }
    checkDataFrameIntegrity();
    if(element.has_value()) {
        DataType value = std::visit([](auto&& value) -> DataType {
            if constexpr (std::is_convertible_v<std::decay_t<decltype(value)>, DataType>) {
                return static_cast<DataType>(value);
            } else {
                throw std::invalid_argument("Incompatible type in variant.");
            }
        }, *element);
        values.insert(values.begin() + index, value);
    }
    else {
        values.insert(values.begin() + index, std::nullopt);
    }
}

template<class DataType>
void Column<DataType>::removeAt(size_t index) {
    if (index >= this->values.size()) throw InvalidIndexException();
    checkDataFrameIntegrity();
    this->values.erase(this->values.begin() + index);
}

template<class DataType>
void Column<DataType>::removeAtFromRow(size_t index) {
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

template<class DataType>
void Column<DataType>::fillNull() {
    for(size_t i = 0; i < this->values.size(); i++) {
        if(!this->values[i].has_value()) {
            this->values[i] = DataType();
        }
    }
}

// END DATA MANIPULATION

// AGGREGATIONS

template<class DataType>
DataType Column<DataType>::min() const {
    if(this->isEmpty()) {
        throw EmptyColumnException();
    }
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    DataType minValue;
    bool foundValidValue = false;
    std::vector<DataType> filteredValues = this->getValues();
    for(const auto& val : filteredValues) {
        if (!foundValidValue || val < minValue) {
            minValue = val;
            foundValidValue = true;
        }
    }
    return minValue;
}

template<class DataType>
DataType Column<DataType>::max() const {
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
double Column<DataType>::mean() const requires DecayedOrDirectNumeric<DataType> {
    if(this->isEmpty()) {
        throw EmptyColumnException();
    }
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    double sum = 0.0;
    size_t count = 0;
    for (const auto& val : this->getValues()) {
        if constexpr (std::is_same_v<DataType, ColumnType>) {
            std::visit([&sum, &count](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (Numeric<T>) {
                    sum += static_cast<double>(v);
                    ++count;
                }
            }, val);
        } else {
            sum += static_cast<double>(val);
            ++count;
        }
    }

    if (count == 0) {
        return std::nan("");
    }
    return sum / count;
}

//template<>
//double Column<ColumnType>::mean() const {
//    if (this->isEmpty()) {
//        throw EmptyColumnException();
//    }
//    if (std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
//        throw NoValidValuesException();
//    }
//    double sum = 0.0;
//    size_t count = 0;
//
//    for (const auto& val : this->getOptionalValues()) {
//        if (val.has_value()) {
//            std::visit(
//                    [&sum, &count](const auto& v) {
//                        using T = std::decay_t<decltype(v)>;
//                        if constexpr (Numeric<T>) {
//                            std::cout << v << std::endl;
//                            sum += v;
//                            ++count;
//                        }
//                    },
//                    *val
//            );
//        }
//    }
//    if(count == 0) {
//        return std::nan("");
//    }
//    return sum / count;
//}


template<class DataType>
double Column<DataType>::median() const requires DecayedOrDirectNumeric<DataType> {
    if(this->isEmpty()) throw EmptyColumnException();
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    std::vector<double> filteredValues;
    for (const auto& val : this->getValues()) {
        if constexpr (std::is_same_v<DataType, ColumnType>) {
            std::visit([&filteredValues](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (Numeric<T>) {
                    filteredValues.push_back(static_cast<double>(v));
                }
            }, val);
        } else {
            filteredValues.push_back(static_cast<double>(val));
        }
    }
    if (filteredValues.empty()) return std::nan("");
    std::sort(filteredValues.begin(), filteredValues.end());

    size_t size = filteredValues.size();
    if (size % 2 == 0) {
        return (filteredValues[size / 2 - 1] + filteredValues[size / 2]) / 2.0;
    }
    return filteredValues[size / 2];
}

template<class DataType>
double Column<DataType>::std() const requires DecayedOrDirectNumeric<DataType> {
    if(this->isEmpty()) throw EmptyColumnException();
    if(std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
        throw NoValidValuesException();
    }
    double mean = this->mean();
    double ssq = 0.0;
    size_t count = 0;
    for (const auto& val : this->getValues()) {
        if constexpr (std::is_same_v<DataType, ColumnType>) {
            std::visit([&ssq, &mean, &count](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (Numeric<T>) {
                    ssq += (static_cast<double>(v) - mean) * (static_cast<double>(v) - mean);
                    ++count;
                }
            }, val);
        } else {
            ssq += (static_cast<double>(val) - mean) * (static_cast<double>(val) - mean);
            ++count;
        }
    }

    if (count < 2) {
        return std::nan("");
    }

    return std::sqrt(ssq / (count - 1));
}

//template<>
//double Column<ColumnType>::std() const {
//    if (this->isEmpty()) throw EmptyColumnException();
//    if (std::none_of(this->values.begin(), this->values.end(), [](const auto& val) { return val.has_value(); })) {
//        throw NoValidValuesException();
//    }
//
//    double mean = this->mean();
//    double ssq = 0.0;
//    size_t validCount = 0;
//
//    for (const auto& val : this->getOptionalValues()) {
//        if (val.has_value()) {
//            double numericValue = std::visit([](const auto& v) -> double {
//                if constexpr (std::is_arithmetic_v<decltype(v)>) {
//                    return static_cast<double>(v);
//                }
//                return 0.0;
//            }, *val);
//
//            ssq += (numericValue - mean) * (numericValue - mean);
//            ++validCount;
//        }
//    }
//    return std::sqrt(ssq / (validCount - 1));
//}


template<class DataType>
double Column<DataType>::var() const requires DecayedOrDirectNumeric<DataType> {
    return std::pow(this->std(), 2);
}

template<class DataType>
DataType Column<DataType>::percentile(double p) const requires DecayedOrDirectNumeric<DataType> {
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
double Column<DataType>::skewness() const requires DecayedOrDirectNumeric<DataType> {
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
DataType Column<DataType>::range() const requires DecayedOrDirectNumeric<DataType> {
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
int Column<DataType>::countNonNull() const {
    return this->getValues().size();
}

template<class DataType>
int Column<DataType>::countNull() const {
    return this->getOptionalValues().size() - this->getValues().size();
}

template<class DataType>
int Column<DataType>::countDistinct() const {
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
std::vector<size_t> Column<DataType>::histogram(size_t numberOfBins) const requires DecayedOrDirectNumeric<DataType> {
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
    for(size_t i = 0; i < filteredValues.size() - 1; i++) {
        result += filteredValues[i] + linker;
    }
    result += filteredValues[filteredValues.size() - 1];
    return result;
}

// END STRING BASED METHODS

// SORT

template<class DataType>
Column<DataType> Column<DataType>::sort(bool ascending) requires Sortable<DataType> {
    Column<DataType> copy = Column<DataType>(*this);
    if(ascending) {
        std::sort(copy.values.begin(), copy.values.end());
    }
    else {
        std::sort(copy.values.begin(), copy.values.end(), std::greater<>());
    }
    return copy;
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
Column<DataType> Column<DataType>::applyOperation(const DataType& value, std::function<DataType(const DataType&, const DataType&)> op) const requires DecayedOrDirectNumeric<DataType> {
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
Column<DataType> Column<DataType>::operator+(const DataType& value) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(value, std::plus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator-(const DataType& value) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(value, std::minus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator*(const DataType& value) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(value, std::multiplies<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator/(const DataType& value) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(value, std::divides<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::applyOperation(const Column<DataType>& other, std::function<DataType(const DataType&, const DataType&)> op) const requires DecayedOrDirectNumeric<DataType> {
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
Column<DataType> Column<DataType>::operator+(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(other, std::plus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator-(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(other, std::minus<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator*(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(other, std::multiplies<DataType>());
}

template<class DataType>
Column<DataType> Column<DataType>::operator/(const Column<DataType>& other) const requires DecayedOrDirectNumeric<DataType> {
    return applyOperation(other, std::divides<DataType>());
}

template<class DataType>
std::optional<DataType> Column<DataType>::operator[](size_t index) const {
    if (index >= values.size()) {
        throw InvalidIndexException();
    }
    return values[index];
}


//int main() {
//    Column<int> col("n");
//    col.add(1);
//    col.add(2);
//    col.add(1);
//    col.add(16);
//    col.add(std::nullopt);
//    col.add(4);
//    col.add(11, 2);
//    col.print();
//    auto evenFilter = [](int value) { return value % 2 == 0; };
//    Column<int> filteredCol = col.filter(evenFilter);
//    filteredCol.print();
//    col.print();
//    Column<int> col1("Column1");
//    col1.add(1);
//    col1.add(2);
//    col1.add(4);
//    col1.print();
//
//    Column<int> col2("Column2");
//    col2.add(5);
//    col2.add(std::nullopt); // Add a null value
//    col2.add(3);
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
//}
