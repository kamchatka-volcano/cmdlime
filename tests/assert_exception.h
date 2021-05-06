#pragma once
#include <gtest/gtest.h>
#include <functional>

template<typename ExceptionType>
void assert_exception(std::function<void()> throwingCode, std::function<void(const ExceptionType&)> exceptionContentChecker)
{
    try{
        throwingCode();
        FAIL() << "exception wasn't thrown!";
    }
    catch(const ExceptionType& e){
        exceptionContentChecker(e);
    }
    catch(...){
        FAIL() << "Unexpected exception was thrown";
    }
}
