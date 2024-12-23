#ifndef ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H
#define ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

class ColumnException : public std::runtime_error {
public:
    explicit ColumnException(const std::string& message)
            : std::runtime_error(message), msg(message) {}

    // Override what() to return the custom message
    virtual const char* what() const noexcept override {
        return msg.c_str();
    }

protected:
    std::string msg;
};

class EmptyColumnException : public ColumnException {
public:
    EmptyColumnException()
            : ColumnException("Column is empty and cannot be processed.") {}

    // Optionally override what() if you need more customization
    const char* what() const noexcept override {
        return "EmptyColumnException: Column is empty and cannot be processed.";
    }
};

class InvalidIndexException : public ColumnException {
public:
    InvalidIndexException()
            : ColumnException("Invalid index accessed in column.") {}

    const char* what() const noexcept override {
        return "InvalidIndexException: Invalid index accessed in column.";
    }
};

class NoValidValuesException : public ColumnException {
public:
    NoValidValuesException()
            : ColumnException("No valid values in column.") {}

    const char* what() const noexcept override {
        return "NoValidValuesException: No valid values in column.";
    }
};

#endif //ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H
