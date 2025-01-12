#ifndef ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H
#define ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

class ColumnException : public std::runtime_error {
public:
    explicit ColumnException(const std::string& message)
            : std::runtime_error(message), msg(message) {}

    virtual const char* what() const noexcept override {
        return msg.c_str();
    }

    ~ColumnException() override = default;

protected:
    std::string msg;
};

class EmptyColumnException : public ColumnException {
public:
    EmptyColumnException()
            : ColumnException("Column is empty and cannot be processed.") {}

    const char* what() const noexcept override {
        return "EmptyColumnException: Column is empty and cannot be processed.";
    }

    ~EmptyColumnException() override = default;
};

class InvalidIndexException : public ColumnException {
public:
    InvalidIndexException()
            : ColumnException("Invalid index accessed in column.") {}

    const char* what() const noexcept override {
        return "InvalidIndexException: Invalid index accessed in column.";
    }

    ~InvalidIndexException() override = default;
};

class NoValidValuesException : public ColumnException {
public:
    NoValidValuesException()
            : ColumnException("No valid values in column.") {}

    const char* what() const noexcept override {
        return "NoValidValuesException: No valid values in column.";
    }

    ~NoValidValuesException() override = default;
};

class InvalidTypeException : public ColumnException {
public:
    InvalidTypeException() : ColumnException("Provided type is not compatible with DataType of the Column") {}

    const char* what() const noexcept override {
        return "InvalidTypeException: Provided type is not compatible with DataType of the Column";
    }

    ~InvalidTypeException() override = default;
};

class InvalidSizeException : public ColumnException {
public:
    InvalidSizeException() : ColumnException("Invalid column size") {}

    const char* what() const noexcept override {
        return "InvalidSizeException. Invalid column size";
    }

    ~InvalidSizeException() override = default;
};

class DataFrameIntegrityException : public ColumnException {
public:
    DataFrameIntegrityException() : ColumnException("Can't modify a column that's part of a DataFrame") {}

    const char* what() const noexcept override {
        return "DataFrameIntegrityException. Can't modify a column that's part of a DataFrame";
    }

    ~DataFrameIntegrityException() override = default;
};

// -----------------

class DataFrameException : public std::runtime_error {
public:
    explicit DataFrameException(const std::string& message)
            : std::runtime_error(message), msg(message) {}

    virtual const char* what() const noexcept override {
        return msg.c_str();
    }

    ~DataFrameException() override = default;

protected:
    std::string msg;
};

class InvalidNumberOfColumnsException : public DataFrameException {
public:
    InvalidNumberOfColumnsException()
            : DataFrameException("DataFrame has invalid number of columns.") {}

    const char* what() const noexcept override {
        return "InvalidNumberOfColumnsException: DataFrame has invalid number of columns.";
    }

    ~InvalidNumberOfColumnsException() override = default;
};

class InvalidNameException : public DataFrameException {
public:
    InvalidNameException()
            : DataFrameException("Column with that name doesn't exist in the DataFrame.") {}

    const char* what() const noexcept override {
        return "InvalidNameException: Column with that name doesn't exist in the DataFrame.";
    }

    ~InvalidNameException() override = default;
};

class TypeMismatchException : public DataFrameException {
public:
    TypeMismatchException()
            : DataFrameException("Column type mismatch.") {}

    const char* what() const noexcept override {
        return "TypeMismatchException: Column type mismatch.";
    }

    ~TypeMismatchException() override = default;
};

#endif //ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H
