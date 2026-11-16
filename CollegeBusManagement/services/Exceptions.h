#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

/**
 * @brief Base exception class for College Bus Management System.
 * 
 * Demonstrates OOP Exception Handling & Inheritance with std::exception.
 */
class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown when an ID already exists (Duplicate ID).
 */
class DuplicateIdException : public AppException {
public:
    explicit DuplicateIdException(const std::string& id, const std::string& entityType = "Record")
        : AppException(entityType + " with ID '" + id + "' already exists! Please use a unique ID.") {}
};

/**
 * @brief Exception thrown when a record is not found.
 */
class NotFoundException : public AppException {
public:
    explicit NotFoundException(const std::string& id, const std::string& entityType = "Record")
        : AppException(entityType + " with ID '" + id + "' was not found.") {}
};

/**
 * @brief Exception thrown when trying to allocate a seat in an already full bus.
 */
class BusFullException : public AppException {
public:
    explicit BusFullException(const std::string& busId, int capacity)
        : AppException("Bus " + busId + " is completely FULL (Capacity: " + std::to_string(capacity) + "). Cannot allocate more students!") {}
};

/**
 * @brief Exception thrown when a file read/write error occurs.
 */
class FileIOException : public AppException {
public:
    explicit FileIOException(const std::string& filename, const std::string& operation)
        : AppException("File handling error: Unable to " + operation + " file '" + filename + "'.") {}
};

/**
 * @brief Exception thrown for invalid input data.
 */
class ValidationException : public AppException {
public:
    explicit ValidationException(const std::string& field, const std::string& reason)
        : AppException("Validation error on '" + field + "': " + reason) {}
};

#endif // EXCEPTIONS_H
