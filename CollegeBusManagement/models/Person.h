#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

/**
 * @brief Abstract Base Class representing a Person.
 * 
 * Demonstrates OOP Concepts:
 * 1. Abstraction - Pure virtual functions define interface without implementation.
 * 2. Encapsulation - Protected member variables with public accessors and mutators.
 * 3. Polymorphism - Virtual functions allow polymorphic behavior in derived classes.
 * 4. Virtual Destructor - Ensures proper cleanup of derived class objects.
 */
class Person {
protected:
    std::string id;
    std::string name;
    std::string phone;
    std::string email;

public:
    // Default Constructor
    Person() : id(""), name(""), phone(""), email("") {}

    // Parameterized Constructor
    Person(const std::string& pId, const std::string& pName, 
           const std::string& pPhone, const std::string& pEmail = "")
        : id(pId), name(pName), phone(pPhone), email(pEmail) {}

    // Virtual Destructor (Essential for Polymorphism)
    virtual ~Person() = default;

    // Getters (Encapsulation)
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getPhone() const { return phone; }
    std::string getEmail() const { return email; }

    // Setters (Encapsulation with validation where appropriate)
    void setId(const std::string& pId) { id = pId; }
    void setName(const std::string& pName) { name = pName; }
    void setPhone(const std::string& pPhone) { phone = pPhone; }
    void setEmail(const std::string& pEmail) { email = pEmail; }

    // Pure Virtual Functions (Abstraction & Polymorphism)
    virtual std::string getRole() const = 0;
    virtual std::string getDetails() const = 0;
    virtual std::string serialize() const = 0;
};

#endif // PERSON_H
