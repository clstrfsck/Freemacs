#ifndef _MINTEXCEPTION_H
#define _MINTEXCEPTION_H

#include <exception>

class MintException : public std::exception {
public:
    MintException(const std::string& err) : _what(err) {}
    virtual ~MintException() throw() { }
    virtual const char* what() const throw() { return _what.c_str(); }
private:
    std::string _what;
}; // MintException

#endif // MINTEXCEPTION_H

// EOF
