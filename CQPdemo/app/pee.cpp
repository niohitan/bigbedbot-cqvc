#include <sstream>
#include <iomanip>
#include <regex>
#include "cqp.h"
#include "pee.h"
#include "appmain.h"
#include "group.h"
#include "cpp-base64/base64.h"

namespace pee {

void modifyCurrency(int64_t qq, int64_t c)
{
    db.exec("UPDATE pee SET currency=? WHERE qqid=?", { c, qq });
}
void modifyBoxCount(int64_t qq, int64_t c)
{
    db.exec("UPDATE pee SET cases=? WHERE qqid=?", { c, qq });
}
void modifyDrawTime(int64_t qq, time_t c)
{
    db.exec("UPDATE pee SET dailytime=? WHERE qqid=?", { c, qq });
}
void modifyKeyCount(int64_t qq, int64_t c)
{
    db.exec("UPDATE pee SET keys=? WHERE qqid=?", { c, qq });
}
int64_t nosmoking(int64_t group, int64_t target, int duration)
{
    if (duration < 0) return -1;
    if (grp::groups.find(group) != grp::groups.end())
    {
        if (grp::groups[group].haveMember(target))
        {
            if (grp::groups[group].members[target].permission >= 2) return -2;
            CQ_setGroupBan(ac, group, target, int64_t(duration) * 60);
            if (duration > 0) smokeGroups[target][group] = time(nullptr) + int64_t(duration) * 60;
            else if (duration == 0) smokeGroups[target].erase(group);
            return duration;
        }
    }
    else
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, target, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            std::string decoded = base64_decode(std::string(cqinfo));
            if (!decoded.empty())
            {
                if (getPermissionFromGroupInfoV2(decoded.c_str()) >= 2) return -2;
                CQ_setGroupBan(ac, group, target, int64_t(duration) * 60);
                if (duration > 0) smokeGroups[target][group] = time(nullptr) + int64_t(duration) * 60;
                else if (duration == 0) smokeGroups[target].erase(group);
                return duration;
            }
        }
    }
    return -1;
}

std::string nosmokingWrapper(int64_t qq, int64_t group, int64_t target, int64_t cost)
{
    int duration = (int)cost;
    if (duration > 30 * 24 * 60) duration = 30 * 24 * 60;
    cost *= cost; // ������ ����

    if (cost < 0) return "��᲻�����ˣ�";

    if (cost > plist[qq].currency) return std::string(CQ_At(qq)) + "��������㣬��Ҫ" + std::to_string(cost) + "����";

    if (cost == 0)
    {
        nosmoking(group, target, duration);
        return "�����";
    }

    double reflect = randReal();
    if (reflect < 0.1) return "��ͻȻ���ˣ�����ʧ��";
    else if (reflect < 0.3)
    {
        auto ret = nosmoking(group, qq, duration);
        if (ret > 0)
        {
            return "���Լ������";
        }
        else if (ret == -2)
        {
            return "��ͻȻ���ˣ�����ʧ��";
        }
        else if (ret == 0 || ret == -1)
        {
            return "��᲻�����ˣ�";
        }
    }
    else
    {
        auto ret = nosmoking(group, target, duration);
        if (ret > 0)
        {
            plist[qq].currency -= cost;
            modifyCurrency(qq, plist[qq].currency);
            std::stringstream ss;
            if (qq == target)
                ss << "���Ҵ���û����������Ҫ������" << cost << "����";
            else
                ss << "����Ŷ������" << cost << "����";
            return ss.str();
        }
        else if (ret == 0 || ret == -1)
        {
            return "��᲻�����ˣ�";
        }
        else if (ret == -2)
        {
            return "���̹�������ϵȺ��Ŷ";
        }
    }
    return "��᲻�����ˣ�";
}

std::tuple<bool, int, time_t> updateStamina(int64_t qq, int cost, bool extra)
{
    time_t t = time(nullptr);
    time_t last = staminaRecoveryTime[qq];
    int stamina = MAX_STAMINA;
    if (last > t) stamina -= (last - t) / STAMINA_TIME + !!((last - t) % STAMINA_TIME);

    bool enough = false;

    if (cost > 0)
    {
        if (staminaExtra[qq] >= cost)
        {
            enough = true;
            staminaExtra[qq] -= cost;
        }
        else if (stamina + staminaExtra[qq] >= cost)
        {
            enough = true;
            cost -= staminaExtra[qq];
            staminaExtra[qq] = 0;
            stamina -= cost;
        }
        else
        {
            enough = false;
        }
    }
    else if (cost < 0)
    {
        enough = true;
        if (stamina - cost <= MAX_STAMINA)
        {
            // do nothing
        }
        else if (extra) // part of cost(recovery) goes to extra
        {
            staminaExtra[qq] += stamina - cost - MAX_STAMINA;
        }
    }

    if (enough)
    {
        if (last > t)
            staminaRecoveryTime[qq] += STAMINA_TIME * cost;
        else
            staminaRecoveryTime[qq] = t + STAMINA_TIME * cost;
    }

    if (enough && stamina >= MAX_STAMINA) staminaRecoveryTime[qq] = t;
    return { enough, stamina, staminaRecoveryTime[qq] - t };
}

std::tuple<bool, int, time_t> testStamina(int64_t qq, int cost)
{
    time_t t = time(nullptr);
    time_t last = staminaRecoveryTime[qq];
    int stamina = MAX_STAMINA;
    if (last > t) stamina -= (last - t) / STAMINA_TIME + !!((last - t) % STAMINA_TIME);

    bool enough = false;

    if (cost > 0)
    {
        if (staminaExtra[qq] >= cost)
        {
            enough = true;
        }
        else if (stamina + staminaExtra[qq] >= cost)
        {
            enough = true;
        }
        else
        {
            enough = false;
        }
    }

    if (enough)
    {
        if (last > t)
            last += STAMINA_TIME * cost;
        else
            last = t + STAMINA_TIME * cost;
    }

    if (enough && stamina >= MAX_STAMINA) last = t;
    return { enough, stamina, last - t };
}

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
    case commands::��ͨ��ʾ:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "�����Ѿ�ע�����";

            return "����Ҫ��ͨ���ˣ���᲻�Ὺͨ����";
        };
        break;
    case commands::��ͨ:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) != plist.end()) return std::string(CQ_At(qq)) + "�����Ѿ�ע�����";

            plist[qq].currency = INITIAL_BALANCE;
            db.exec("INSERT INTO pee(qqid, currency, cases, dailytime, keys) VALUES(? , ? , ? , ? , ?)",
                { qq, plist[qq].currency, 0, 0, 0 });

            std::stringstream ss;
            ss << "����Կ�ʼ�����ˣ��͸���" << plist[qq].currency << "����";
            return ss.str();
        };
        break;
    case commands::���:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
            std::stringstream ss;
            ss << CQ_At(qq) << "��������Ϊ" << plist[qq].currency << "������"
                << plist[qq].keys << "��Կ��";
            auto[enough, stamina, rtime] = updateStamina(qq, 0);
            ss << "\n�㻹��" << stamina + staminaExtra[qq] << "������";
            if (stamina < MAX_STAMINA)
                ss << "����������" << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
            return ss.str();
        };
        break;
    case commands::����:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (grp::groups.find(group) != grp::groups.end())
            {
                if (grp::groups[group].haveMember(CQ_getLoginQQ(ac)))
                {
                    if (grp::groups[group].members[CQ_getLoginQQ(ac)].permission < 2)
                        return "";
                }
            }
            else
            {
                const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, CQ_getLoginQQ(ac), FALSE);
                if (cqinfo && strlen(cqinfo) > 0)
                {
                    std::string decoded = base64_decode(std::string(cqinfo));
                    if (!decoded.empty())
                    {
                        if (getPermissionFromGroupInfoV2(decoded.c_str()) < 2)
                            return "";
                    }
                }
            }

            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            int64_t target = 0;
            if (true || args.size() == 1)
            {
                if (prevUser.find(group) == prevUser.end()) return "";
                target = prevUser[group];
            }

            int64_t cost = -1;
            try {
                if (args.size() >= 2)
                    cost = std::stoll(args[1]);
            }
            catch (std::exception&) {
                //ignore
            }

            return nosmokingWrapper(qq, group, target, cost);

        };
        break;
    case commands::���:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (grp::groups.find(group) != grp::groups.end())
            {
                if (grp::groups[group].haveMember(CQ_getLoginQQ(ac)))
                {
                    if (grp::groups[group].members[CQ_getLoginQQ(ac)].permission < 2)
                        return "";
                }
            }
            else
            {
                const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, CQ_getLoginQQ(ac), FALSE);
                if (cqinfo && strlen(cqinfo) > 0)
                {
                    std::string decoded = base64_decode(std::string(cqinfo));
                    if (!decoded.empty())
                    {
                        if (getPermissionFromGroupInfoV2(decoded.c_str()) < 2)
                            return "";
                    }
                }
            }

            if (args.size() < 1) return "";
            try {
                //unsmoke(std::stoll(args[1]));
                CQ_setGroupBan(ac, group, std::stoll(args[1]), 0);
                return "";
            }
            catch (std::exception&) {}
            return "";
        };
        break;
    case commands::����:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            time_t lastDailyTime = plist[qq].last_draw_time;
            if (lastDailyTime > daily_refresh_time)
            {
                std::stringstream ss;
                ss << CQ_At(qq) << "��������Ѿ�����ˣ���������8";
                return ss.str();
            }

            std::stringstream ss;
            ss << CQ_At(qq) << "��������쵽" << FREE_BALANCE_ON_NEW_DAY << "����";
            if (remain_daily_bonus)
            {
                int bonus = randInt(1, remain_daily_bonus > 66 ? 66 : remain_daily_bonus);
                remain_daily_bonus -= bonus;
                plist[qq].currency += FREE_BALANCE_ON_NEW_DAY + bonus;

                ss << "�����������ȵ���" << bonus << "����\n"
                    << "�������ػ�ʣ" << remain_daily_bonus << "����";
            }
            else
            {
                plist[qq].currency += FREE_BALANCE_ON_NEW_DAY;
                ss << "\nÿ������ô���ˣ�������ȵ�";
            }
            plist[qq].last_draw_time = time(nullptr);
            modifyCurrency(qq, plist[qq].currency);
            modifyDrawTime(qq, plist[qq].last_draw_time);
            return ss.str();
            /*
            if (plist[qq].last_draw_time > daily_time_point)
            if (remain_daily_bonus > 0)
            {
                int bonus = randInt(0, remain_daily_bonus);
                remain_daily_bonus -= bonus;
            }
            */
            return "";
        };
        break;
    case commands::����:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            auto[enough, stamina, rtime] = updateStamina(qq, 1);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "������������㣬��������"
                << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";

            plist[qq].currency += 1;
            modifyCurrency(qq, plist[qq].currency);

            return std::string(CQ_At(qq)) + "����1�������õ�1����";
        };
        break;
    case commands::����:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "��׼��";

            std::stringstream ss;
            if (plist[qq].keys >= 1)
            {
                plist[qq].keys--;
                modifyKeyCount(qq, plist[qq].keys);
            }
            else if (plist[qq].currency >= FEE_PER_CASE)
            {
                auto [enough, stamina, rtime] = updateStamina(qq, 1);
                if (!enough)
                {
                    ss << CQ_At(qq) << "������������㣬��������"
                        << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
                    return ss.str();
                }
                plist[qq].currency -= FEE_PER_CASE;
            }
            else
                return std::string(CQ_At(qq)) + "���������";

            const case_detail& reward = draw_case(randReal());
            if (reward > 300) ss << "��Ӵ��" << CQ_At(qq) << "���ˣ�������";
            else ss << CQ_At(qq) << "����ϲ�㿪����";
            ss << reward.full() << "�����" << reward.worth() << "����";

            plist[qq].currency += reward.worth();
            if (plist[qq].currency < 0) plist[qq].currency = 0;
            modifyCurrency(qq, plist[qq].currency);
            modifyBoxCount(qq, ++plist[qq].opened_box_count);
            //ss << "�㻹��" << stamina << "��������";

            return ss.str();
        };
        break;
    case commands::����10:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "��׼��";

            std::stringstream ss;
            if (plist[qq].keys >= 10)
            {
                plist[qq].keys -= 10;
                modifyKeyCount(qq, plist[qq].keys);
            }
            else if (plist[qq].currency >= FEE_PER_CASE * 10)
            {
                auto [enough, stamina, rtime] = updateStamina(qq, 10);
                if (!enough)
                {
                    ss << CQ_At(qq) << "������������㣬��������"
                        << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
                    return ss.str();
                }
                plist[qq].currency -= FEE_PER_CASE * 10;
            }
            else
                return std::string(CQ_At(qq)) + "���������";

            std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
            int r = 0;
            /*
            for (size_t i = 0; i < 10; ++i)
            {
                const case_detail& reward = draw_case(randReal());
                case_counts[reward.type_idx()]++;
                r += reward.worth();
            }
            if (r > 300) ss << "��Ӵ��" << CQ_At(qq) << "���ˣ�������";
            else ss << CQ_At(qq) << "����ϲ�㿪����";
            for (size_t i = 0; i < case_counts.size(); ++i)
            {
                if (case_counts[i])
                {
                    ss << case_counts[i] << "��" <<
                        ((i == CASE_POOL.size()) ? CASE_DEFAULT.name() : CASE_POOL[i].name()) << "��";
                }
            }
            ss << "һ��" << r << "����";
            */

            ss << CQ_At(qq) << "����ϲ�㿪���ˣ�\n";
            for (size_t i = 0; i < 10; ++i)
            {
                const case_detail& reward = draw_case(randReal());
                case_counts[reward.type_idx()]++;
                r += reward.worth();
                ss << "- " << reward.full() << " (" << reward.worth() << "��)\n";
            }
            ss << "������";
            for (size_t i = 0; i < case_counts.size(); ++i)
            {
                if (case_counts[i])
                {
                    ss << case_counts[i] << "��" <<
                        ((i == CASE_POOL.size()) ? CASE_DEFAULT.name() : CASE_POOL[i].name()) << "��";
                }
            }
            ss << "һ��" << r << "����";

            plist[qq].currency += r;
            if (plist[qq].currency < 0) plist[qq].currency = 0;
            plist[qq].opened_box_count += 10;
            modifyCurrency(qq, plist[qq].currency);
            modifyBoxCount(qq, plist[qq].opened_box_count);

            return ss.str();
        };
        break;
    case commands::������:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "��׼��";

            if (plist[qq].currency < COST_OPEN_RED)
                return std::string(CQ_At(qq)) + "���������";

            auto[enough, stamina, rtime] = updateStamina(qq, COST_OPEN_RED_STAMINA);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "������������㣬��������"
                << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
            else
            {
                plist[qq].currency -= COST_OPEN_RED;
                ss << getCard(group, qq) << "����" << COST_OPEN_RED << "������" << COST_OPEN_RED_STAMINA <<"�������������ܣ�\n";

                std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
                int count = 0;
                int cost = 0;
                int res = 0;
                case_detail reward;
                do {
                    ++count;
                    cost += FEE_PER_CASE;
                    reward = draw_case(randReal());
                    case_counts[reward.type_idx()]++;
                    res += reward.worth();
                    plist[qq].currency += reward.worth() - FEE_PER_CASE;
                    plist[qq].opened_box_count++;

                    if (reward.type_idx() == 2)
                    {
                        ss << CQ_At(qq) << "����" << count << "���������ڿ�����" << reward.full() << " (" << reward.worth() << "��)��"
                            << "���ξ�����" << res - cost - COST_OPEN_RED << "����";
                        if (plist[qq].currency < 0) plist[qq].currency = 0;
                        modifyCurrency(qq, plist[qq].currency);
                        modifyBoxCount(qq, plist[qq].opened_box_count);
                        return ss.str();
                    }
                    else
                        if (reward.type_idx() == 1)
                        {
                            ss << "��Ӵ��" << CQ_At(qq) << "���ˣ�����" << count << "�����Ӿ�Ȼ������" << reward.full() << " (" << reward.worth() << "��)��"
                                << "���ξ�����" << res - cost - COST_OPEN_RED << "����";
                            if (plist[qq].currency < 0) plist[qq].currency = 0;
                            modifyCurrency(qq, plist[qq].currency);
                            modifyBoxCount(qq, plist[qq].opened_box_count);
                            return ss.str();
                        }
                } while (plist[qq].currency >= FEE_PER_CASE);

                ss << CQ_At(qq) << "�Ʋ��ˣ�����" << count << "������Ҳû�ܿ������䣬"
                    << "���ξ�����" << res - cost - COST_OPEN_RED << "����";
                if (plist[qq].currency < 0) plist[qq].currency = 0;
                modifyCurrency(qq, plist[qq].currency);
                modifyBoxCount(qq, ++plist[qq].opened_box_count);
                return ss.str();

                //ss << "�㻹��" << stamina << "��������";
            }

            return ss.str();
        };
        break;
    case commands::������:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "��׼��";

            if (plist[qq].currency < COST_OPEN_YELLOW)
                return std::string(CQ_At(qq)) + "���������";

            auto[enough, stamina, rtime] = updateStamina(qq, MAX_STAMINA);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "������������㣬��������"
                << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
            else
            {
                plist[qq].currency -= COST_OPEN_YELLOW;
                ss << getCard(group, qq) << "����" << COST_OPEN_YELLOW << "������ȫ�������������ܣ�\n";

                std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
                int count = 0;
                int cost = 0;
                int res = 0;
                case_detail reward;
                do {
                    ++count;
                    cost += FEE_PER_CASE;
                    reward = draw_case(randReal());
                    case_counts[reward.type_idx()]++;
                    res += reward.worth();
                    plist[qq].currency += reward.worth() - FEE_PER_CASE;
                    plist[qq].opened_box_count++;

                    if (reward.type_idx() == 1)
                    {
                        ss << CQ_At(qq) << "����" << count << "���������ڿ�����" << reward.full() << " (" << reward.worth() << "��)��"
                            << "���ξ�����" << res - cost - COST_OPEN_YELLOW << "����";
                        if (plist[qq].currency < 0) plist[qq].currency = 0;
                        modifyCurrency(qq, plist[qq].currency);
                        modifyBoxCount(qq, plist[qq].opened_box_count);
                        return ss.str();
                    }
                } while (plist[qq].currency >= FEE_PER_CASE);

                ss << CQ_At(qq) << "�Ʋ��ˣ�����" << count << "������Ҳû�ܿ������䣬"
                    << "���ξ�����" << res - cost - COST_OPEN_YELLOW << "����";
                if (plist[qq].currency < 0) plist[qq].currency = 0;
                modifyCurrency(qq, plist[qq].currency);
                modifyBoxCount(qq, plist[qq].opened_box_count);
                return ss.str();

                //ss << "�㻹��" << stamina << "��������";
            }

            return ss.str();
        };
        break;

    case commands::����endless:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            return "���̨��Ⱥ��͵�ˣ�û������";
        };
        break;
        /*
    case commands::����endless:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";
            //CQ_setGroupBan(ac, group, qq, 60);
            //return "��׼��";

            if (plist[qq].currency < COST_OPEN_ENDLESS)
                return std::string(CQ_At(qq)) + "���������";

            auto [enough, stamina, rtime] = updateStamina(qq, COST_OPEN_ENDLESS_STAMINA);

            std::stringstream ss;
            if (!enough) ss << CQ_At(qq) << "������������㣬��������"
                << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
            else
            {
                plist[qq].currency -= COST_OPEN_ENDLESS;
                long long total_cases = plist[qq].currency / FEE_PER_CASE;
                plist[qq].currency %= FEE_PER_CASE;
                long long extra_cost = 0.1 * total_cases * FEE_PER_CASE;
                total_cases -= long long(std::floor(total_cases * 0.1));
                ss << getCard(group, qq) << "����" << COST_OPEN_ENDLESS + extra_cost << "������" << COST_OPEN_ENDLESS_STAMINA << "�������������ܣ�\n";
                std::vector<int> case_counts(CASE_POOL.size() + 1, 0);
                int count = 0;
                int cost = 0;
                int res = 0;
                case_detail max;
                case_detail reward;
                do {
                    ++count;
                    cost += FEE_PER_CASE;
                    reward = draw_case(randReal());
                    case_counts[reward.type_idx()]++;
                    res += reward.worth();
                    plist[qq].currency += reward.worth();
                    plist[qq].opened_box_count++;

                    if (max.worth() < reward.worth()) max = reward;
                } while (--total_cases > 0);

                ss << CQ_At(qq) << "�����������" << count << "�����ӣ�������" << case_counts[1] << "�����䣬" << case_counts[2] << "�����䣬" << case_counts[0] << "�����䣬\n"
                    << "��ֵǮ����" << max.full() << " (" << max.worth() << "��)��"
                    << "���ξ�����" << res - cost - COST_OPEN_ENDLESS - extra_cost << "����";
                if (plist[qq].currency < 0) plist[qq].currency = 0;
                modifyCurrency(qq, plist[qq].currency);
                modifyBoxCount(qq, plist[qq].opened_box_count);
                return ss.str();

                //ss << "�㻹��" << stamina << "��������";
            }

            return ss.str();
        };
        break;
        */
    default: break;
    }

    return c;
}

command smokeIndicator(const char* msg)
{
    if (msg == nullptr) return command();
    std::stringstream ss(msg);
    std::vector<std::string> query;
    while (ss)
    {
        std::string s;
        ss >> s;
        if (!s.empty())
            query.push_back(s);
    }
    if (query.empty()) return command();

    auto cmd = query[0];
    if (cmd.length() <= 4) return command();
    if (!std::regex_match(cmd.substr(0, 4), std::regex(R"(��(��|��))"))) return command();

    command c;
    c.args = query;
    c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
    {
        if (args.size() == 1)
        {
            return "";
        }
        auto cmd = args[0];

        std::string targetName = cmd.substr(4);
        int64_t target = 0;

        // @
        if (std::smatch res; std::regex_match(targetName, res, std::regex(R"(\[CQ:at,qq=(\d+)\])")))
            try {
                target = std::stoll(res[0]);
            }
            catch (std::exception&) {
                //ignore
            }

        // qqid_str
        else if (qqid_str.find(targetName) != qqid_str.end())
            target = qqid_str[targetName];

        // nick, card
        else
            if (grp::groups.find(group) != grp::groups.end())
            {
                if (int64_t qq = grp::groups[group].getMember(targetName.c_str()); qq != 0)
                    target = qq;
            }

        int64_t cost = -1;
        try {
            cost = std::stoll(args[1]);
        }
        catch (std::exception&) {
            //ignore
        }

        return nosmokingWrapper(qq, group, target, cost);
    };

    return c;
}

void peeCreateTable()
{
    if (db.exec(
        "CREATE TABLE IF NOT EXISTS pee( \
            qqid    INTEGER PRIMARY KEY NOT NULL, \
            currency INTEGER            NOT NULL, \
            cases   INTEGER             NOT NULL, \
            dailytime INTEGER           NOT NULL,  \
            keys    INTEGER             NOT NULL  \
         )") != SQLITE_OK)
        CQ_addLog(ac, CQLOG_ERROR, "pee", db.errmsg());
}

void peeLoadFromDb()
{
    auto list = db.query("SELECT * FROM pee", 5);
    for (auto& row : list)
    {
        int64_t qq = std::any_cast<int64_t>(row[0]);
        int64_t p1, p2, p4;
        time_t  p3;
        p1 = std::any_cast<int64_t>(row[1]);
        p2 = std::any_cast<int64_t>(row[2]);
        p3 = std::any_cast<time_t> (row[3]);
        p4 = std::any_cast<int64_t>(row[4]);
        plist[qq] = { p1, p2, p3, p4 };
        plist[qq].freeze_assets_expire_time = INT64_MAX;
    }
    char msg[128];
    sprintf(msg, "added %u users", plist.size());
    CQ_addLog(ac, CQLOG_DEBUG, "pee", msg);
}

const double UNSMOKE_COST_RATIO = 3;
std::string unsmoke(int64_t qq)
{
    if (smokeGroups.find(qq) == smokeGroups.end() || smokeGroups[qq].empty())
        return "��ô�ñ��̰�";

    if (plist.find(qq) == plist.end()) return "�㻹û�п�ͨ����";

    time_t t = time(nullptr);
    int total_remain = 0;
    for (auto& g : smokeGroups[qq])
    {
        if (t <= g.second)
        {
            int remain = (g.second - t) / 60; // min
            int extra = (g.second - t) % 60;  // sec
            total_remain += remain + !!extra;
        }
    }
    std::stringstream ss;
    ss << "����" << smokeGroups[qq].size() << "��Ⱥ����" << total_remain << "���ӣ�";
    int total_cost = total_remain * UNSMOKE_COST_RATIO;
    if (plist[qq].currency < total_cost)
    {
        ss << "������������Ҫ" << total_cost << "��������";
    }
    else
    {
        plist[qq].currency -= total_cost;
        ss << "���νӽ�����" << total_cost << "����";
        modifyCurrency(qq, plist[qq].currency);
        for (auto& g : smokeGroups[qq])
        {
            if (t > g.second) continue;
            std::string qqname = getCard(g.first, qq);
            CQ_setGroupBan(ac, g.first, qq, 0);
            std::stringstream sg;
            int remain = (g.second - t) / 60; // min
            int extra = (g.second - t) % 60;  // sec
            sg << qqname << "���Ѿ���" << (long long)std::round((remain + !!extra) * UNSMOKE_COST_RATIO) << "��������ӽ��ɹ�";
            CQ_sendGroupMsg(ac, g.first, sg.str().c_str());
        }
        smokeGroups[qq].clear();
    }
    return ss.str();
}

void flushDailyTimep(bool autotriggered)
{
    daily_refresh_time = time(nullptr);
    daily_refresh_tm = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN);
    if (autotriggered) daily_refresh_tm_auto = daily_refresh_tm;

    remain_daily_bonus = DAILY_BONUS + extra_tomorrow;
    extra_tomorrow = 0;

    broadcastMsg("ÿ������ˢ���ˣ���");
    CQ_addLog(ac, CQLOG_DEBUG, "pee", std::to_string(daily_refresh_time).c_str());
}

const case_detail& draw_case(double p)
{
    size_t idx = 0;

    if (p >= 0.0 && p <= 1.0) 
    {
        double totalp = 0;
        for (const auto& c : CASE_POOL)
        {
            if (p <= totalp + c.prob()) break;
            ++idx;
            totalp += c.prob();
        }
        // idx = CASE_POOL.size() if not match any case
    }

    size_t detail_idx = randInt(0, CASE_DETAILS[idx].size() - 1);
    return CASE_DETAILS[idx][detail_idx];
}

}
