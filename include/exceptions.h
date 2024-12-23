#ifndef ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H
#define ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

// Base class for custom column-related exceptions
class ColumnException : public std::runtime_error {
public:
    explicit ColumnException(const std::string& message) : std::runtime_error(message) {}
};

// Exception for when the column is empty
class EmptyColumnException : public ColumnException {
public:
    EmptyColumnException() : ColumnException("Column is empty and cannot be processed.") {}
};

// Exception for when an invalid index is accessed
class InvalidIndexException : public ColumnException {
public:
    InvalidIndexException() : ColumnException("Invalid index accessed in column.") {}
};


#endif //ABSTRACTPROGRAMMINGPROJECT_EXCEPTIONS_H
