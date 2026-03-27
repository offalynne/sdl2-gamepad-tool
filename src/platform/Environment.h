#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QString>
#include <memory>

class Environment
{
public:
    virtual ~Environment() = default;

    virtual QString get(const char *key) = 0;
    virtual void set(const char *key, const char *value) = 0;
    virtual void unset(const char *key) = 0;

    static std::unique_ptr<Environment> create();
};

#endif // ENVIRONMENT_H
