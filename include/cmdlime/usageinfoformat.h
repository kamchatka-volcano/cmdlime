#ifndef CMDLIME_USAGEINFOFORMAT_H
#define CMDLIME_USAGEINFOFORMAT_H

namespace cmdlime{

struct UsageInfoFormat{
    int terminalWidth = 80;
    int maxNameColumnWidth = 46;
    int columnsSpacing = 4;
    int nameIndentation = 4;
};

}

#endif //CMDLIME_USAGEINFOFORMAT_H