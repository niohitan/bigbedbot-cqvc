#include <sstream>
#include "cqp.h"
#include "eat.h"
#include "appmain.h"
#include "cpp-base64/base64.h"
#include "private/qqid.h"
#include "group.h"
#include <Windows.h>

namespace eat {

std::string food::to_string(int64_t group)
{
    std::string qqname = "����Ⱥ��";
    if (group != 0 && offererType == food::QQ && offerer.qq != 0)
    {
        if (grp::groups.find(group) != grp::groups.end())
        {
            if (grp::groups[group].haveMember(offerer.qq))
                qqname = grp::groups[group].members[offerer.qq].card;
        }
        else
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, offerer.qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                //CQ_addLog(ac, CQLOG_DEBUG, "eat", cqinfo);
                std::string decoded = base64_decode(std::string(cqinfo));
                if (!decoded.empty())
                {
                    qqname = getCardFromGroupInfoV2(decoded.c_str());
                }
                else
                    CQ_addLog(ac, CQLOG_ERROR, "eat", "groupmember base64 decode error");
            }
        }
    }

    std::stringstream ss;
    ss << name;
    switch (offererType)
    {
    case food::NAME:  ss << " (" << offerer.name << "�ṩ)"; break;
    case food::QQ:    ss << " (" << qqname << "�ṩ)"; break;
    default: break;
    }

    //std::stringstream ssd;
    //ssd << offererType << ": " << name << " | " << offerer.name << " | " << offerer.qq;
    //CQ_addLog(ac, CQLOG_DEBUG, "eat", ssd.str().c_str());
    return ss.str();
}

inline int addFood(food& f)
{
    const char query[] = "INSERT INTO food(name, adder, qq) VALUES (?,?,?)";
    int ret;
    auto nameutf8 = gbk2utf8(f.name);
    auto offererutf8 = gbk2utf8(f.offerer.name);
    switch (f.offererType)
    {
    case food::NAME:        ret = db.exec(query, { nameutf8, offererutf8, nullptr });  break;
    case food::QQ:          ret = db.exec(query, { nameutf8, nullptr, f.offerer.qq });    break;
    case food::ANONYMOUS:   ret = db.exec(query, { nameutf8, nullptr, nullptr });         break;
    default: break;
    }
    if (ret != SQLITE_OK)
    {
        std::stringstream ss;
        ss << "addFood: " << db.errmsg() << ", " << f.name;
        CQ_addLog(ac, CQLOG_ERROR, "eat", ss.str().c_str());
        return 1;
    }
    foodList.push_back(f);
    return 0;
}

inline food& getFood()
{
    return foodList[randInt(0, foodList.size() - 1)];
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
    case commands::��ʲô:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (foodList.empty()) return "�o";
            std::stringstream ss;
            ss << CQ_At(qq);
            ss << "��������ѡ��";
            ss << getFood().to_string(group);
            return ss.str();
        };
        break;
    case commands::��ʲô:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            return "��֪��";
        };
        break;
    case commands::��ʲô:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            return "��ʲô";
        };
        break;
    case commands::��ʲôʮ��:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (foodList.empty()) return "�o";
            std::stringstream ss;
            ss << CQ_At(qq);
            ss << "��������ѡ��\n";
            for (size_t i = 0; i < 10; ++i)
                ss << " - " << getFood().to_string(group) << "\n";
            ss << "�Զ�����ʮ����ף����������";
            return ss.str();
        };
        break;
    case commands::�Ӳ�:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (args.size() == 1) return "�������ܳԵ�";

            std::string r(raw);
            r = strip(r.substr(5));    // �Ӳˣ�BC D3 / B2 CB / 20
            if (raw.empty()) return "�������ܳԵ�";
            if (r == "����") return "�������ܳԵ�";
            if (r.length() > 30) return "����Ƿ�����������ˣ�";

            //TODO filter

            // check repeat
            for (auto& f : foodList)
            {
                if (f.name == r)
                    return f.name + "�Ѿ����ˣ�����";
            }

            food f;
            f.name = r;
            f.offerer.qq = qq;
            f.offererType = f.QQ;
            if (addFood(f))
            {
                return "��׼��";
            }
            std::stringstream ss;
            ss << "�����" << f.to_string(group);
            return ss.str();
        };
        break;
    case commands::�˵�:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (foodList.empty()) return "��";
            // defuault: last 9 entries
            size_t range_min = (foodList.size() <= 9) ? 0 : (foodList.size() - 9);
            size_t range_max = (foodList.size() <= 9) ? (foodList.size() - 1) : (range_min + 8);

            // arg[1] is range_mid
            if (args.size() > 1 && foodList.size() > 9) try
            {
                int tmp = std::stoi(args[1]) - 1 - 4;
                if (tmp < 0)
                {
                    range_min = 0;
                    range_max = 8;
                }
                else
                {
                    range_min = tmp;
                    range_max = range_min + 8;
                }

                if (range_max >= foodList.size()) {
                    range_max = foodList.size() - 1;
                    range_min = range_max - 8;
                }

            }
            catch (std::invalid_argument) {}

            if (range_min > range_max) return "";
            std::stringstream ret;
            for (size_t i = range_min; i <= range_max; ++i)
            {
                ret << i + 1 << ": " << foodList[i].to_string(group);
                if (i != range_max) ret << '\n';
            }

            return ret.str();
        };
        break;
    case commands::ɾ��:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (qq != qqid_admin)
                return "��׼ɾ";

            if (db.exec(
                "DELETE FROM food \
            ") != SQLITE_OK)
            {
                CQ_addLog(ac, CQLOG_ERROR, "eat", db.errmsg());
                return db.errmsg();
            }
            foodList.clear();
            return "drop��";
        };
        break;
    default: break;
    }

    return c;
}
void foodCreateTable()
{
    if (db.exec(
        "CREATE TABLE IF NOT EXISTS food( \
            id    INTEGER PRIMARY KEY AUTOINCREMENT, \
            name  TEXT            NOT NULL,      \
            adder TEXT,                          \
            qq    INTEGER                        \
         )") != SQLITE_OK)
        CQ_addLog(ac, CQLOG_ERROR, "eat", db.errmsg());
}

void foodLoadListFromDb()
{
    auto list = db.query("SELECT * FROM food", 4);
    for (auto& row : list)
    {
        food f;
        f.name = utf82gbk(std::any_cast<std::string>(row[1]));
        if (row[2].has_value())
        {
            f.offererType = f.NAME;
            f.offerer.name = utf82gbk(std::any_cast<std::string>(row[2]));
        }
        else if (row[3].has_value())
        {
            f.offererType = f.QQ;
            f.offerer.qq = std::any_cast<int64_t>(row[3]);
        }
        else
            f.offererType = f.ANONYMOUS;
        foodList.push_back(f);
    }
    char msg[128];
    sprintf(msg, "added %u foods", foodList.size());
    CQ_addLog(ac, CQLOG_DEBUG, "eat", msg);
}

}
