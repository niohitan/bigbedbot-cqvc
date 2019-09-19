#include "help.h"
#include <sstream>
#include <algorithm>
namespace help
{
    const std::vector<std::string> changelog{
        {R"(20190919
Fix: �¼�[�ں�ҩˮ]����ų���0
)"},
        {R"(20190916
Fix: ���⿪�䣨���ԣ�3����+�����һ�Σ�
Mod: �¼�[������] ����->0.5%
Mod: �¼�[���в���] ����->0.1%
)"},
        {R"(20190909
Del: �¼�[ѭ��ҩˮ]����ʵ��һ�ξ�ɾ�ˣ�
Add: ���⿪�䣨���ԣ�3����+�����һ�Σ�
)"},
        {R"(20190902
Fix: ����û��
Mod: �ں�ҩˮ�ų�0
Add: �¼�[��������Ԥ��]
)"},
        {R"(20190830
Add: changelog
Add: �¼�[������]
Add: �¼�[���в���]
Mod: ���ô���Ϊ5
Mod: ��ճ������Ḳ������Ч��
Fix: δע����˿���ҡ��
)"},
    };

    std::string boot_info()
    {
        std::stringstream ss;
        ss << "bot���ˣ�";
        ss << help(1);
        return ss.str();
    }

    std::string help(unsigned count)
    {
        std::stringstream ss;
        ss << "���������ڣ�" << __TIMESTAMP__ << std::endl;
        ss << "�޸ļ�¼��" << std::endl;
        for (unsigned i = 0; i < count && i < changelog.size(); ++i)
        {
            ss << changelog[i];
        }
        std::string ret = ss.str();
        while (ret.back() == '\n' || ret.back() == 'r')
        {
            while (ret.back() == '\n') ret.pop_back();
            while (ret.back() == '\r') ret.pop_back();
        }
        return ret;
    }
}
