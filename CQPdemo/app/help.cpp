#include "help.h"
#include <sstream>
#include <algorithm>
namespace help
{
    std::string boot_info()
    {
        std::stringstream ss;
        ss << "bot���ˣ�";
        ss << help();
        return ss.str();
    }

    std::string help()
    {
        std::stringstream ss;
        ss << "���������ڣ�" << __TIMESTAMP__ << std::endl <<
            "�����ĵ���https://github.com/yaasdf/bigbedbot-cqvc/blob/master/CQPdemo/README.md";
        return ss.str();
    }
}
