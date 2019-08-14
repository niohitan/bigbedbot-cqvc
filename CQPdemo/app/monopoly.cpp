#include "monopoly.h"
#include "cqp.h"
#include "appmain.h"
#include "private/qqid.h"

namespace mnp
{
const std::vector<event_type> EVENT_POOL{
    { 0.1,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "Կ��1��"; }},
    { 0.1,[](int64_t group, int64_t qq) { nosmoking(group, qq, 1); return "���̶�ͯ�ײͣ�����1����"; }},
    { 0.05,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "Կ���������2��Կ��"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 10; modifyKeyCount(qq, plist[qq].keys); return "Կ����װ�����10��Կ��"; }},
    { 0.04,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "����ˮ�����1����"; }},
    { 0.03,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "�洫����ˮ��������ֻ���1����"; }},
    { 0.03,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "ո�³�������ˮ��������ֻ���1����"; }},
    { 0.07,[](int64_t group, int64_t qq) { plist[qq].currency += 2; modifyCurrency(qq, plist[qq].currency); return "�����濵˧��ţ���棬���2����"; }},
    { 0.05,[](int64_t group, int64_t qq) { plist[qq].currency += 5; modifyCurrency(qq, plist[qq].currency); return "צ�ӵ����ģ�ͣ����5����"; }},
    { 0.03,[](int64_t group, int64_t qq) { plist[qq].currency += 20; modifyCurrency(qq, plist[qq].currency); return "Ⱥ����ֽ�����20����"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].currency += 100; modifyCurrency(qq, plist[qq].currency); return "�󴲼�����ᣬ���100����"; }},
    { 0.06,[](int64_t group, int64_t qq) { updateStamina(qq, -1); return "����һ��ȯ�����1������"; }},
    { 0.03,[](int64_t group, int64_t qq) { updateStamina(qq, -2); return "����һ��ȯ�����2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, -4); return "ԭ��ƿ�����4������"; }},
    { 0.005,[](int64_t group, int64_t qq) { updateStamina(qq, -pee::MAX_STAMINA); return "�ظ�ʯ����о�������ˬ������������"; }},
    { 0.01,[](int64_t group, int64_t qq) { nosmoking(group, qq, 5); return "���̰����ײͣ�����5����"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        auto [enough, stamina, t] = updateStamina(qq, 0);
        updateStamina(qq, stamina);
        return "�Ӱ��ʼ������������������";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        updateStamina(qq, -3, true);
        using namespace std::string_literals;
        return "С���������3������"s + (pee::staminaExtra[qq] > 0 ? "������ͻ������������"s : ""s);
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 50;
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        return "�����ƣ�����ֱ������������ѣ�����ҽҩ��50��";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 10; 
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        pee::extra_tomorrow += 10;
        return "���Ʋ��뽱������10��������������"; 
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        double p = randReal(0.5, 1.5);
        plist[qq].currency *= p;
        modifyCurrency(qq, plist[qq].currency);
        using namespace std::string_literals;
        return "һ�Ѳ��ˣ�����������"s + std::to_string(p) + "��"s;
    }},

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qqid_dk, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            nosmoking(group, qqid_dk, 1);
            return "����Ҳ������������";
        }
        else return EVENT_POOL[0].func()(group, qq);
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                auto [enough, stamina, t] = updateStamina(qq, 0);
                updateStamina(qq, stamina);
                updateStamina(qq, -2);
            }
        }
        return "���򲭣���Ⱥ�����˵������������2��";
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                plist[qq].currency *= 0.9;
                modifyCurrency(qq, plist[qq].currency);
            }
        }
        return "��ʯ����Ⱥ�����˵�Ǯ������ը��10%";
    }},

    { 0.002,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 500;
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        return "JJ�֣���ļұ�ը���ˣ���ʧ500��";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency += 10;
        modifyCurrency(qq, plist[qq].currency);
        pee::extra_tomorrow -= 10;
        if (-pee::extra_tomorrow > pee::DAILY_BONUS) pee::extra_tomorrow = -pee::DAILY_BONUS;
        return "����ǯ����֤������������͵��10��";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::thread([]() {using namespace std::chrono_literals; std::this_thread::sleep_for(2s); pee::flushDailyTimep(); }).detach();
        return "F5";
    }},

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        for (size_t i = 0; i < size; ++i)
        {
            unsigned idx = randInt(0, numbers.size() - 1);
            p = p * 10 + numbers[idx];
            numbers.erase(numbers.begin() + idx);
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "����ҩˮ���������������";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        unsigned idx = randInt(0, numbers.size() - 1);
        for (size_t i = 0; i < size; ++i)
        {
            p = p * 10 + numbers[idx];
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "�ں�ҩˮ�������ȫ�������һ��������";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        for (size_t i = 0; i < size; ++i)
        {
            p = p * 10 + numbers[i];
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "����ҩˮ�����������������";
    } },


    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                updateStamina(qq, -pee::MAX_STAMINA);
            }
        }
        return "һ��ƽ���ܲ�����Ⱥ�����˵�������������";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (plist[qq].keys < 1) return EVENT_POOL[0].func()(group, qq);
        plist[qq].keys--; 
        modifyKeyCount(qq, plist[qq].keys);
        return "�����ӣ���ʲôҲû�п���������1��Կ��"; 
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                int bonus = randInt(50, 1000);
                plist[qq].currency += bonus;
                modifyCurrency(qq, plist[qq].currency);
            }
        }
        return "��紵����Ⱥ��һ�������Լ������񵽶���Ŷ";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qqid_dk, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            int d = randInt(1, 5);
            nosmoking(group, qq, d);
            using namespace std::string_literals;
            return "�������ӣ��㱻���ӽ���"s + std::to_string(d) + "����"s;
        }
        else return EVENT_POOL[0].func()(group, qq);
    } },

    { 0.01,[](int64_t group, int64_t qq) { return "��Կ��һ�ѣ��㳢���������䵫��ʧ����"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "��е�����һ��������û��ʲô��"; }},
    { 0.01,[](int64_t group, int64_t qq) { return "�޴�س���һ��������û��ʲô��"; }},
    { 0.01,[](int64_t group, int64_t qq) { return "һ��ϡ�����壬����û��ʲô��"; } },
    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HAMMER + "������" + EMOJI_HAMMER; } },
};
const event_type EVENT_DEFAULT{ 1.0, [](int64_t group, int64_t qq) { return "������ʲô��ô��"; } };


command msgDispatcher(const char* msg)
{
    command c;
    auto query = msg2args(msg);
    if (query.empty()) return c;

    auto cmd = query[0];
    if (commands_str.find(cmd) == commands_str.end()) return c;

    c.args = query;
    switch (c.c = commands_str[cmd])
    {
    case commands::�鿨:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            auto [enough, stamina, rtime] = updateStamina(qq, 1);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "������������㣬��������"
                << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
            else
            {
                auto reward = draw_event(randReal());
                ss << CQ_At(qq) << "����ϲ��鵽��" << reward.func()(group, qq);
            }
            return ss.str();
        };
        break;

    default:
        break;
    }
    return c;
}

const event_type& draw_event(double p)
{
    if (p < 0.0 || p > 1.0) return EVENT_DEFAULT;
    double totalp = 0;
    for (const auto& c : EVENT_POOL)
    {
        if (p <= totalp + c.prob()) return c;
        totalp += c.prob();
    }
    return EVENT_DEFAULT;        // not match any case
}

}