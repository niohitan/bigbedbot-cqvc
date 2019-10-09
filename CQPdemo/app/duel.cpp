#include "duel.h"
#include "cqp.h"
#include "appmain.h"
#include <sstream>
#include <set>
#include <thread>
#include "group.h"
using namespace std::string_literals;
std::string str_put_fail = "��᲻��"s + EMOJI_DOWN + "ע��";
std::string str_nobody = "�o��"s + EMOJI_DOWN + "ע��ȡ������";
const char* cstr_put_fail = str_put_fail.c_str();
const char* cstr_nobody = str_nobody.c_str();

namespace duel
{

command msgDispatcher(const char* msg)
{
    command c;
    auto query = msg2args(msg);
    if (query.empty()) return c;

    auto cmd = query[0];

    c.args = query;

    if (cmd.substr(0, 4) == "ҡ��" && cmd.length() > 4 && query.size() > 1)
    {
        cmd = "ҡ��";
        c.args.resize(3);
        c.args[0] = cmd;
        c.args[1] = query[0].substr(4);
        c.args[2] = query[1];
    }
    else if (cmd.substr(0, 4) != "ҡ��" && cmd.substr(0, 2) == "ҡ" && cmd.length() > 2 && query.size() > 1)
    {
        cmd = "ҡ��";
        c.args.resize(3);
        c.args[0] = cmd;
        c.args[1] = query[0].substr(2);
        c.args[2] = query[1];
    }
    if (commands_str.find(cmd) == commands_str.end()) return c;

    switch (c.c = commands_str[cmd])
    {
    case commands::flipcoin:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (pee::plist.find(qq) == pee::plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            flipcoin::roundStart(group);
            if (args.size() > 2)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[2]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                if (p <= 0) return "";
                else if (args[1] == "��") flipcoin::put(group, qq, { p, 0 });
                else if (args[1] == "��") flipcoin::put(group, qq, { 0, p });
            }
            return "";
        };
        break;
    case commands::��:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (pee::plist.find(qq) == pee::plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            if (args.size() > 1)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[1]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                if (p <= 0) return "";
                flipcoin::put(group, qq, { p, 0 });
                return "";
            }
            return cstr_put_fail;
        };
        break;
    case commands::��:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (pee::plist.find(qq) == pee::plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            if (args.size() > 1)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[1]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                if (p <= 0) return "";
                flipcoin::put(group, qq, { 0, p });
                return "";
            }
            return cstr_put_fail;
        };
        break;


    case commands::roulette:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (pee::plist.find(qq) == pee::plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            roulette::roundStart(group);
            if (args.size() > 2)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[2]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                std::string subc = args[1];
                if (p <= 0 || roulette::gridTokens.find(subc) == roulette::gridTokens.end()) return "";
                else roulette::put(group, qq, roulette::gridTokens.at(subc), p);
            }
            return "";
        };
        break;

    case commands::ҡ��:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw) -> std::string
        {
            if (pee::plist.find(qq) == pee::plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            if (args.size() > 2)
            {
                int64_t p = 0;
                try {
                    p = std::stoll(args[2]);
                }
                catch (std::exception&) {}
                if (p < 0) p = 0;

                std::string subc = args[1];
                if (p <= 0 || roulette::gridTokens.find(subc) == roulette::gridTokens.end()) return "";
                else roulette::put(group, qq, roulette::gridTokens.at(subc), p);
            }
            return "";
        };
        break;

    default:
        break;
    }
    return c;
}

///////////////////////////////////////////////////////////////////////////////
// flipcoin

namespace flipcoin
{

void roundStart(int64_t group)
{
    if (grp::groups[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "There is already a game running at this group.");
        return;
    }

    CQ_sendGroupMsg(ac, group, "��һ�ֵķ�����ʼ�ˣ���ȺԱӻԾ�μ�");
    grp::groups[group].flipcoin_running = true;
    grp::groups[group].flipcoin_game = {};

    grp::groups[group].flipcoin_game.startTime = time(nullptr);
    std::thread([=]() {
        using namespace std::chrono_literals;

        // 40s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 20s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 10s
        std::this_thread::sleep_for(10s);
        roundAnnounce(group);

        // 5s
        std::this_thread::sleep_for(5s);
        //flipcoin::roundAnnounce(group);

        // end
        std::this_thread::sleep_for(5s);
        roundEnd(group);

        }).detach();
}

void roundAnnounce(int64_t group)
{
    if (!grp::groups[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "No flipcoin round is running at this group.");
        return;
    }

    std::stringstream ss;
    const auto& r = grp::groups[group].flipcoin_game;
    ss << "���ַ���" << EMOJI_DOWN << "עʱ�仹ʣ" << r.startTime + 60 - time(nullptr) << "�룬��ǰ" << EMOJI_DOWN << "ע�����\n";

    ss << "�ܼ�" << r.total << "����������" << r.front << "��������" << r.back << "��";
    if (r.total > 0)
    {
        ss << "\n";
        ss << "���棺";
        for (const auto& [qq, stat] : r.pee_per_player)
        {
            if (stat.front)
                ss << getCard(group, qq) << "(" << stat.front << "����) "
                << "(" << 50.0 * stat.front / r.front << "%)  ";
        }
        ss << "\n";
        ss << "���棺";
        for (const auto& [qq, stat] : r.pee_per_player)
        {
            if (stat.back)
                ss << getCard(group, qq) << "(" << stat.back << "����) "
                << "(" << 50.0 * stat.back / r.back << "%)  ";
        }
    }
    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}

void roundEnd(int64_t group)
{
    if (!grp::groups[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "No flipcoin round is running at this group.");
        return;
    }

    const auto& r = grp::groups[group].flipcoin_game;
    if (r.front == 0 || r.back == 0)
    {
        CQ_sendGroupMsg(ac, group, cstr_nobody);
        roundCancel(group);
        return;
    }

    bool front = randInt(0, 1);
    double per = randReal();

    int64_t deno = front ? r.front : r.back;
    double totalper = 0;
    int64_t winner;

    if (deno != 0)
    {
        for (const auto& [qq, bet] : r.pee_per_player)
        {
            totalper += 1.0 * (front ? bet.front : bet.back) / deno;
            if (totalper >= per)
            {
                winner = qq;
                break;
            }
        }
    }

    std::stringstream ss;
    ss << "���ַ������������Ϊ" << (front ? "����" : "����") << "��";
    if (deno != 0)
    {
        auto& w = r.pee_per_player.at(winner);
        int64_t bet = (front ? w.front : w.back);
        ss << CQ_At(winner) << "��" << 50.0 * bet / deno
            << "%�ĸ���Ӯ���˴�" << r.total << "������";
    }
    else
        ss << r.total << "����ô����";
    CQ_sendGroupMsg(ac, group, ss.str().c_str());

    pee::plist[winner].currency += r.total;
    modifyCurrency(winner, pee::plist[winner].currency);
    grp::groups[group].flipcoin_running = false;
}

void roundCancel(int64_t group)
{
    if (!grp::groups[group].flipcoin_running)
        return;

    for (const auto& [qq, stat] : grp::groups[group].flipcoin_game.pee_per_player)
    {
        pee::plist[qq].currency += stat.front;
        pee::plist[qq].currency += stat.back;
        modifyCurrency(qq, pee::plist[qq].currency);
    }

    grp::groups[group].flipcoin_running = false;
    CQ_sendGroupMsg(ac, group, "�������ˣ�����������");
}

void put(int64_t group, int64_t qq, bet bet)
{
    if (!grp::groups[group].flipcoin_running)
    {
        CQ_sendGroupMsg(ac, group, "��Ⱥô�ÿ��̰�");
        return;
    }

    if (bet.front <= 0 && bet.back <= 0)
    {
        CQ_sendGroupMsg(ac, group, "����Ǹ����֣�");
        return;
    }

    if (pee::plist[qq].currency < bet.front + bet.back)
    {
        std::string s = CQ_At(qq) + "���������";
        CQ_sendGroupMsg(ac, group, s.c_str());
        return;
    }

    auto& round = grp::groups[group].flipcoin_game;
    auto& player = round.pee_per_player[qq];
    player.front += bet.front;
    player.back += bet.back;

    round.front += bet.front;
    round.back += bet.back;
    round.total += bet.front + bet.back;

    pee::plist[qq].currency -= bet.front + bet.back;
    modifyCurrency(qq, pee::plist[qq].currency);

    std::stringstream ss;
    if (bet.front)
    {
        if (player.front == bet.front)
            ss << getCard(group, qq) << "�ɹ�" << EMOJI_DOWN << "ע" << "����" << bet.front << "����";
        else
            ss << getCard(group, qq) << "�ɹ�" << EMOJI_DOWN << "ע" << "����" << "��" << player.front << "����";
    }
    else if (bet.back)
    {
        if (player.back == bet.back)
            ss << getCard(group, qq) << "�ɹ�" << EMOJI_DOWN << "ע" << "����" << bet.back << "����";
        else
            ss << getCard(group, qq) << "�ɹ�" << EMOJI_DOWN << "ע" << "����" << "��" << player.back << "����";
    }

    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}
}


namespace roulette
{

void roundStart(int64_t group)
{
    if (grp::groups[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "There is already a game running at this group.");
        return;
    }

    CQ_sendGroupMsg(ac, group, "��һ�ֵ�ҡ�ſ�ʼ�ˣ���ȺԱӻԾ�μ�");
    grp::groups[group].roulette_running = true;
    grp::groups[group].roulette_game = {};

    grp::groups[group].roulette_game.startTime = time(nullptr);
    std::thread([=]() {
        using namespace std::chrono_literals;

        // 40s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 20s
        std::this_thread::sleep_for(20s);
        roundAnnounce(group);

        // 10s
        std::this_thread::sleep_for(10s);
        roundAnnounce(group);

        // 5s
        std::this_thread::sleep_for(5s);
        //roulette::roundAnnounce(group);

        // end
        std::this_thread::sleep_for(5s);
        roundEnd(group);

        }).detach();
}

void roundAnnounce(int64_t group)
{
    if (!grp::groups[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "No roulette round is running at this group.");
        return;
    }

    std::stringstream ss;
    const auto& r = grp::groups[group].roulette_game;
    ss << "����ҡ��ʱ�仹ʣ" << r.startTime + 60 - time(nullptr) << "�룬��ǰ" << EMOJI_DOWN << "ע�����\n";

    ss << "�ܼ�" << r.total << "����";
    if (r.total > 0)
    {
        /*
        for (size_t i = 0; i < GRID_COUNT; ++i)
            if (r.amount[i])
                ss << "��" << gridName[i] << ": " << r.amount[i];
                */

        for (size_t i = 0; i < GRID_COUNT; ++i)
        {
            if (!r.amount[i]) continue;
            ss << "\n";
            ss << gridName[i] << ": ";
            for (const auto& [qq, stat] : r.pee_per_player)
                if (stat.amount[i])
                    ss << getCard(group, qq) << "("<< stat.amount[i] << "����) ";
        }
    }
    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}

void roundEnd(int64_t group)
{
    if (!grp::groups[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "No roulette round is running at this group.");
        return;
    }

    const auto& r = grp::groups[group].roulette_game;
    if (r.total == 0)
    {
        CQ_sendGroupMsg(ac, group, cstr_nobody);
        roundCancel(group);
        return;
    }

    int result = randInt(0, 36);
    std::set<unsigned> red_idx{ 1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36 };
    std::set<unsigned> blk_idx{ 2, 4, 6, 8, 10, 11, 13, 15, 17, 20, 22, 24, 26, 28, 29, 31, 33, 35 };
    bool b_red = red_idx.find(result) != red_idx.end();
    bool b_black = blk_idx.find(result) != blk_idx.end();
    bool b_odd = result % 2 != 0;
    bool b_even = !b_odd && result != 0;
    bool b_1st = result >= 1 && result <= 12;
    bool b_2nd = result >= 13 && result <= 24;
    bool b_3rd = result >= 25 && result <= 36;

    std::map<int64_t, int64_t> reward;
    for (const auto& [qq, bet] : r.pee_per_player)
    {
        int64_t reward_tmp = 0;
        if (result == 0 && bet.amount[0]) reward_tmp += bet.amount[0] * 50;
        if (result != 0 && bet.amount[result]) reward_tmp += bet.amount[result] * 36;
        if (b_black && bet.amount[Cblack]) reward_tmp += bet.amount[Cblack] * 2;
        if (b_red && bet.amount[Cred]) reward_tmp += bet.amount[Cred] * 2;
        if (b_odd && bet.amount[Aodd]) reward_tmp += bet.amount[Aodd] * 2;
        if (b_even && bet.amount[Aeven]) reward_tmp += bet.amount[Aeven] * 2;
        if (b_1st && bet.amount[P1st]) reward_tmp += bet.amount[P1st] * 3;
        if (b_2nd && bet.amount[P2nd]) reward_tmp += bet.amount[P2nd] * 3;
        if (b_3rd && bet.amount[P3rd]) reward_tmp += bet.amount[P3rd] * 3;
        if (reward_tmp)
        {
            CQ_addLog(ac, CQLOG_DEBUG, "duel", ("reward: "s + std::to_string(qq) + " " + std::to_string(reward_tmp)).c_str());
            reward[qq] = reward_tmp;
            pee::plist[qq].currency += reward[qq];
            modifyCurrency(qq, pee::plist[qq].currency);
        }
    }

    std::stringstream ss;
    ss << "����ҡ�Ž��������Ϊ��"
        << "[" << result << "]";
    if (result != 0)
        ss << (b_black ? " [��]" : " [��]")
            << (b_odd ? " [��]" : " [˫]")
            << (b_1st ? " [1st]" : "")
            << (b_2nd ? " [2nd]" : "")
            << (b_3rd ? " [3rd]" : "");
    ss << "�������У�";
    if (!reward.empty())
        for (auto& [qq, amount] : reward)
            ss << "\n" << CQ_At(qq) << " (" << amount << "����)";
    else
        ss << "\n" << EMOJI_NONE;
    CQ_sendGroupMsg(ac, group, ss.str().c_str());
    grp::groups[group].roulette_running = false;
}

void roundCancel(int64_t group)
{
    if (!grp::groups[group].roulette_running)
        return;

    for (const auto& [qq, stat] : grp::groups[group].roulette_game.pee_per_player)
    {
        for (const auto& a : stat.amount)
            pee::plist[qq].currency += a;
        modifyCurrency(qq, pee::plist[qq].currency);
    }

    grp::groups[group].roulette_running = false;
    CQ_sendGroupMsg(ac, group, "�Ų�ҡ�ˣ�����������");
}

void put(int64_t group, int64_t qq, grid g, int64_t amount)
{
    if (!grp::groups[group].roulette_running)
    {
        CQ_sendGroupMsg(ac, group, "��Ⱥô�ÿ��̰�");
        return;
    }

    if (amount < 0)
    {
        CQ_sendGroupMsg(ac, group, "����Ǹ����֣�");
        return;
    }

    if (pee::plist[qq].currency < amount)
    {
        std::string s = CQ_At(qq) + "���������";
        CQ_sendGroupMsg(ac, group, s.c_str());
        return;
    }

    auto& round = grp::groups[group].roulette_game;
    auto& player = round.pee_per_player[qq];
    player.amount[g] += amount;

    round.amount[g] += amount;
    round.total += amount;

    pee::plist[qq].currency -= amount;
    modifyCurrency(qq, pee::plist[qq].currency);

    std::stringstream ss;
    if (player.amount[g] == amount)
        ss << getCard(group, qq) << "�ɹ�" << EMOJI_DOWN << "ע[" << gridName[g] << "]" << amount << "����";
    else
        ss << getCard(group, qq) << "�ɹ�" << EMOJI_DOWN << "ע[" << gridName[g] << "]��" << player.amount[g] << "����";

    CQ_sendGroupMsg(ac, group, ss.str().c_str());
}
}

}

