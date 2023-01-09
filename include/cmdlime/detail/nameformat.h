#ifndef CMDLIME_NAMEFORMAT_H
#define CMDLIME_NAMEFORMAT_H

#include "gnuformat.h"
#include "simpleformat.h"
#include "posixformat.h"
#include "x11format.h"
#include "external/sfun/utility.h"

namespace cmdlime::detail{

struct NameFormat{
    static std::string name(Format format, const std::string& varName)
    {
        switch (format) {
            case Format::Simple:
                return FormatCfg<Format::Simple>::nameProvider::name(varName);
            case Format::POSIX:
                return FormatCfg<Format::POSIX>::nameProvider::name(varName);
            case Format::X11:
                return FormatCfg<Format::X11>::nameProvider::name(varName);
            case Format::GNU:
                return FormatCfg<Format::GNU>::nameProvider::name(varName);
        }
        sfun::unreachable();
    }

    static std::string shortName(Format format, const std::string& varName)
    {
        switch (format) {
            case Format::Simple:
                return FormatCfg<Format::Simple>::nameProvider::shortName(varName);
            case Format::POSIX:
                return FormatCfg<Format::POSIX>::nameProvider::shortName(varName);
            case Format::X11:
                return FormatCfg<Format::X11>::nameProvider::shortName(varName);
            case Format::GNU:
                return FormatCfg<Format::GNU>::nameProvider::shortName(varName);
        }
        sfun::unreachable();
    }
    static std::string fullName(Format format, const std::string& varName)
    {
        switch (format) {
            case Format::Simple:
                return FormatCfg<Format::Simple>::nameProvider::fullName(varName);
            case Format::POSIX:
                return FormatCfg<Format::POSIX>::nameProvider::fullName(varName);
            case Format::X11:
                return FormatCfg<Format::X11>::nameProvider::fullName(varName);
            case Format::GNU:
                return FormatCfg<Format::GNU>::nameProvider::fullName(varName);
        }
        sfun::unreachable();
    }
    static std::string valueName(Format format, const std::string& type)
    {
        switch (format) {
            case Format::Simple:
                return FormatCfg<Format::Simple>::nameProvider::valueName(type);
            case Format::POSIX:
                return FormatCfg<Format::POSIX>::nameProvider::valueName(type);
            case Format::X11:
                return FormatCfg<Format::X11>::nameProvider::valueName(type);
            case Format::GNU:
                return FormatCfg<Format::GNU>::nameProvider::valueName(type);
        }
        sfun::unreachable();
    }
};

}

#endif //CMDLIME_NAMEFORMAT_H