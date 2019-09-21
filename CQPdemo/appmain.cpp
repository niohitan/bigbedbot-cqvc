/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/

#include "cqp.h"
#include "appmain.h" //Ӧ��AppID����Ϣ������ȷ��д�������Q�����޷�����

#include "app/eat.h"
#include "app/pee.h"
#include "app/duel.h"
#include "app/monopoly.h"
#include "app/help.h"
#include "app/event_case.h"

using namespace std;
int64_t QQME;


/* 
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)() {

	return 0;
}


/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
*/
CQEVENT(int32_t, __eventExit, 0)() {
    if (enabled)
    {
        for (auto& [group, round] : duel::flipcoin::groupStat)
        {
            duel::flipcoin::roundCancel(group);
        }
        for (auto& [group, round] : duel::roulette::groupStat)
        {
            duel::roulette::roundCancel(group);
        }
        pee::db.transactionStop();
        enabled = false;
    }
	return 0;
}

/*
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	enabled = true;
    QQME = CQ_getLoginQQ(ac);
    eat::foodCreateTable();
    eat::foodLoadListFromDb();
    pee::peeCreateTable();
    pee::peeLoadFromDb();
    std::thread(timedCommit, std::ref(eat::db)).detach();
    std::thread(timedCommit, std::ref(pee::db)).detach();

    {
        auto t = time(nullptr);
        t -= 60 * 60 * 24; // yesterday
        pee::daily_refresh_time = t;
        pee::daily_refresh_tm_auto = *localtime(&t);
    }

    std::thread([&]() {
        auto &rec = pee::daily_refresh_tm_auto;
        using namespace std::chrono_literals;
        while (enabled)
        {
            std::this_thread::sleep_for(5s);

            auto t = time(nullptr);
            std::tm tm = *localtime(&t);

            // Skip if same day
            if (tm.tm_year <= rec.tm_year && tm.tm_yday <= rec.tm_yday)
                continue;

            if (tm.tm_hour >= pee::NEW_DAY_TIME_HOUR && tm.tm_hour >= pee::NEW_DAY_TIME_MIN)
                pee::flushDailyTimep(true);
        }
    }).detach();

    std::string boot_info = help::boot_info();
    CQ_sendGroupMsg(ac, 479733965, boot_info.c_str());
    CQ_sendGroupMsg(ac, 391406854, boot_info.c_str());

	return 0;
}


/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)() {
    if (enabled)
    {
        for (auto& [group, round] : duel::flipcoin::groupStat)
        {
            duel::flipcoin::roundCancel(group);
        }
        for (auto& [group, round] : duel::roulette::groupStat)
        {
            duel::roulette::roundCancel(group);
        }
        pee::db.transactionStop();
        enabled = false;
    }
	return 0;
}


/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {

    if (!strcmp(msg, "�ӽ���"))
    {
        CQ_sendPrivateMsg(ac, fromQQ, pee::unsmoke(fromQQ).c_str());
        return EVENT_BLOCK;
    }

	//���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
	//������ظ���Ϣ������֮���Ӧ��/�������������� return EVENT_IGNORE - ���Ա�����Ϣ
	return EVENT_IGNORE;
}

time_t banTime_me = 0;

/*
* Type=2 Ⱥ��Ϣ
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
    
    if (time(NULL) <= banTime_me) return EVENT_IGNORE;

    if (!strcmp(msg, "����"))
    {
        std::string help = help::help();
        CQ_sendGroupMsg(ac, fromGroup, help.c_str());
        return EVENT_BLOCK;
    }

    // ��ʲô
    auto c = eat::msgDispatcher(msg);
    if (c.func) CQ_sendGroupMsg(ac, fromGroup, c.func(fromGroup, fromQQ, c.args, msg).c_str());

    // ����
    auto d = pee::msgDispatcher(msg);
    if (d.func) CQ_sendGroupMsg(ac, fromGroup, d.func(fromGroup, fromQQ, d.args, msg).c_str());

    // ���̵���
    auto e = pee::smokeIndicator(msg);
    if (e.func) CQ_sendGroupMsg(ac, fromGroup, e.func(fromGroup, fromQQ, e.args, msg).c_str());

    // 
    auto f = duel::msgDispatcher(msg);
    if (f.func) CQ_sendGroupMsg(ac, fromGroup, f.func(fromGroup, fromQQ, f.args, msg).c_str());

    // fate
    auto g = mnp::msgDispatcher(msg);
    if (g.func) CQ_sendGroupMsg(ac, fromGroup, g.func(fromGroup, fromQQ, g.args, msg).c_str());

    // event_case
    auto h = event_case::msgDispatcher(msg);
    if (h.func) CQ_sendGroupMsg(ac, fromGroup, h.func(fromGroup, fromQQ, h.args, msg).c_str());

    // update smoke status 
    if (fromQQ != QQME && fromQQ != 10000 && fromQQ != 1000000)
    {
        pee::prevUser[fromGroup] = fromQQ;
        if (pee::smokeGroups.find(fromQQ) != pee::smokeGroups.end())
        {
            time_t t = time(nullptr);
            std::list<int64_t> expired;
            for (auto& g : pee::smokeGroups[fromQQ])
                if (t > g.second) expired.push_back(g.first);
            for (auto& g : expired)
                pee::smokeGroups.erase(g);
        }
    }
    return (c.func || d.func || e.func || f.func || g.func || h.func) ? EVENT_BLOCK : EVENT_IGNORE;
	//return EVENT_BLOCK; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=4 ��������Ϣ
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=101 Ⱥ�¼�-����Ա�䶯
* subType �����ͣ�1/��ȡ������Ա 2/�����ù���Ա
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=102 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/ȺԱ�뿪 2/ȺԱ���� 3/�Լ�(����¼��)����
* fromQQ ������QQ(��subTypeΪ2��3ʱ����)
* beingOperateQQ ������QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=103 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/����Ա��ͬ�� 2/����Ա����
* fromQQ ������QQ(������ԱQQ)
* beingOperateQQ ������QQ(����Ⱥ��QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=201 �����¼�-���������
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=301 ����-�������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=302 ����-Ⱥ���
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
*/
CQEVENT(int32_t, __menuA, 0)() {
    pee::flushDailyTimep();
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
    time_t t = time(nullptr);
    for (auto& c : pee::smokeGroups)
        for (auto& g : c.second)
            if (t < g.second)
                CQ_setGroupBan(ac, g.first, c.first, 0);
	return 0;
}


//////////////////////////////////
std::vector<std::string> msg2args(const char* msg)
{
    std::vector<std::string> query;
    if (msg == nullptr) return query;
    std::stringstream ss(msg);
    while (ss)
    {
        std::string s;
        ss >> s;
        if (!s.empty())
            query.push_back(s);
    }
    return query;
}

#define byte win_byte_override 
#include <Windows.h>
#undef byte
std::string gbk2utf8(std::string gbk)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
    wchar_t wstr[128];
    memset(wstr, 0, sizeof(wstr));

    MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

    char str[256];
    memset(str, 0, sizeof(str));
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);

    return std::string(str);
}

std::string utf82gbk(std::string utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    wchar_t wstr[128];
    memset(wstr, 0, sizeof(wstr));

    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

    char str[256];
    memset(str, 0, sizeof(str));
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);

    return std::string(str);
}

std::string strip(std::string& s)
{
    int end = (int)s.length() - 1;
    int start = 0;
    while (start < (int)s.length() - 1 && (s[start] == ' ')) ++start;
    while (end > start && (s[end] == ' ' || s[end] == '\n' || s[end] == '\r')) --end;
    return s.substr(start, end + 1 - start);
}

// Big endian

GroupMemberInfo::GroupMemberInfo(const char* base64_decoded)
{
    size_t offset = 0;
    int16_t len = 0;

    group = ntohll(*(uint64_t*)(base64_decoded[offset]));
    offset += 8;

    qqid = ntohll(*(uint64_t*)(base64_decoded[offset]));
    offset += 8;

    len = ntohs(*(uint16_t*)(base64_decoded[offset]));
    offset += 2;
    nick = simple_str(len, &base64_decoded[offset]);
    offset += len;

    len = ntohs(*(uint16_t*)(base64_decoded[offset]));
    offset += 2;
    card = simple_str(len, &base64_decoded[offset]);
    offset += len;

    gender = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    age = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    len = ntohs(*(uint16_t*)(base64_decoded[offset]));
    offset += 2;
    area = simple_str(len, &base64_decoded[offset]);
    offset += len;

    joinTime = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    speakTime = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    len = ntohs(*(uint16_t*)(base64_decoded[offset]));
    offset += 2;
    level = simple_str(len, &base64_decoded[offset]);
    offset += len;

    permission = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    dummy1 = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    len = ntohs(*(uint16_t*)(base64_decoded[offset]));
    offset += 2;
    title = simple_str(len, &base64_decoded[offset]);
    offset += len;

    titleExpireTime = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;

    canModifyCard = ntohl(*(uint32_t*)(base64_decoded[offset]));
    offset += 4;
}

std::string getCardFromGroupInfoV2(const char* base64_decoded)
{
    /*
    size_t nick_offset = 8 + 8;
    size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
    size_t card_offset = nick_offset + 2 + nick_len;
    size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
    if (card_len == 0)
        return std::string(&base64_decoded[nick_offset+2], nick_len);
    else
        return std::string(&base64_decoded[card_offset+2], card_len);
        */
    return GroupMemberInfo(base64_decoded).card;
}

int getPermissionFromGroupInfoV2(const char* base64_decoded)
{
    /*
    size_t nick_offset = 8 + 8;
    size_t nick_len = (base64_decoded[nick_offset] << 8) + base64_decoded[nick_offset + 1];
    size_t card_offset = nick_offset + 2 + nick_len;
    size_t card_len = (base64_decoded[card_offset] << 8) + base64_decoded[card_offset + 1];
    size_t area_offset = card_offset + 2 + card_len + 4 + 4;
    size_t area_len = (base64_decoded[area_offset] << 8) + base64_decoded[area_offset + 1];
    size_t glvl_offset = area_offset + 2 + area_len + 4 + 4;
    size_t glvl_len = (base64_decoded[glvl_offset] << 8) + base64_decoded[glvl_offset + 1];
    size_t perm_offset = glvl_offset + 2 + glvl_len;
    return (base64_decoded[perm_offset + 0] << (8 * 3)) +
           (base64_decoded[perm_offset + 1] << (8 * 2)) +
           (base64_decoded[perm_offset + 2] << (8 * 1)) +
           (base64_decoded[perm_offset + 3]);
           */
    return GroupMemberInfo(base64_decoded).permission;
}

#include "cpp-base64/base64.h"
std::string getCard(int64_t group, int64_t qq)
{
    std::string qqname;
    const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
    if (cqinfo && strlen(cqinfo) > 0)
    {
        std::string decoded = base64_decode(std::string(cqinfo));
        if (!decoded.empty())
        {
            qqname = getCardFromGroupInfoV2(decoded.c_str());
        }
    }
    if (qqname.empty()) qqname = CQ_At(qq);
    return qqname;
}