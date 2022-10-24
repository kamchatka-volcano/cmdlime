#ifndef CMDLIME_COMMANDLINEREADERPTR_H
#define CMDLIME_COMMANDLINEREADERPTR_H

namespace cmdlime::detail {
class ICommandLineReader;


class CommandLineReaderPtr {
    friend class ICommandLineReader;

private:
    CommandLineReaderPtr(ICommandLineReader* reader)
        : reader_{reader}
    {}

public:
    CommandLineReaderPtr() = default;

    const ICommandLineReader* operator->() const
    {
        return reader_;
    }

    ICommandLineReader* operator->()
    {
        return reader_;
    }

    const ICommandLineReader& operator*() const
    {
        return *reader_;
    }

    ICommandLineReader& operator*()
    {
        return *reader_;
    }

    operator bool() const
    {
        return reader_;
    }

private:
    ICommandLineReader* reader_ = nullptr;
};

}

#endif //CMDLIME_COMMANDLINEREADERPTR_H