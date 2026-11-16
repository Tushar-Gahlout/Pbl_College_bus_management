#ifndef USER_H
#define USER_H

#include <string>
#include <sstream>
#include <vector>

/**
 * @brief User model for authentication and role-based access.
 */
class User {
private:
    std::string username;
    std::string password;
    std::string role; // "Admin", "Student", "Driver"
    std::string associatedId; // Links to Student ID or Driver ID
    std::string fullName;

public:
    User() : username(""), password(""), role("Student"), associatedId(""), fullName("") {}

    User(const std::string& uName, const std::string& pwd,
         const std::string& r, const std::string& assocId = "",
         const std::string& fName = "")
        : username(uName), password(pwd), role(r), associatedId(assocId), fullName(fName) {}

    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }
    std::string getRole() const { return role; }
    std::string getAssociatedId() const { return associatedId; }
    std::string getFullName() const { return fullName; }

    void setUsername(const std::string& u) { username = u; }
    void setPassword(const std::string& p) { password = p; }
    void setRole(const std::string& r) { role = r; }
    void setAssociatedId(const std::string& id) { associatedId = id; }
    void setFullName(const std::string& name) { fullName = name; }

    bool validatePassword(const std::string& pwd) const {
        return password == pwd;
    }

    std::string serialize() const {
        return username + "|" + password + "|" + role + "|" + associatedId + "|" + fullName;
    }

    static User deserialize(const std::string& line) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }
        if (tokens.size() < 3) return User();
        std::string assoc = (tokens.size() >= 4) ? tokens[3] : "";
        std::string full = (tokens.size() >= 5) ? tokens[4] : "";
        return User(tokens[0], tokens[1], tokens[2], assoc, full);
    }
};

#endif // USER_H
