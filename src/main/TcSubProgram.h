/*
 * ToyCompile 子程序基类。
 * 创建于 2022年9月28日。
 */

#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>

/**
 * ToyCompile 子程序基类。
 * 
 * 子程序进入点为 run 函数。
 */

class TcSubProgram {

public:

    virtual void printUsage(std::ostream& out) = 0;

    virtual int run(
        std::map<std::string, std::string>& paramMap,
        std::set<std::string>& paramSet,
        std::vector<std::string>& additionalValues,
        std::istream& in,
        std::ostream& out
    ) = 0;

    virtual ~TcSubProgram() = default;

protected:



};
