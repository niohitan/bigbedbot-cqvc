#define CQAPPID "com.yaasdf.bigbedbot" //���޸�AppID������� http://d.cqp.me/Pro/����/������Ϣ
#define CQAPPINFO CQAPIVERTEXT "," CQAPPID

inline bool enabled = false;

#include "app/dbconn.h"
enum enumCQBOOL
{
    FALSE,
    TRUE
};
extern int64_t QQME;

extern time_t banTime_me;

////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <chrono>
inline void timedCommit(SQLite& db)
{
    db.transactionStart();
    while (enabled)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1min);
        db.commit(true);
    }
    db.transactionStop();
}

////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>
#include <sstream>
std::vector<std::string> msg2args(const char* msg);
std::string gbk2utf8(std::string gbk);
std::string utf82gbk(std::string utf8);


////////////////////////////////////////////////////////////////////////////////
#include <random>
inline std::random_device random_rd;  // �����ڻ����������������
inline std::mt19937 random_gen(random_rd()); // �� rd() ���ֵı�׼ mersenne_twister_engine

inline int randInt(int min, int max)   // [min, max]
{
    std::uniform_int_distribution<> dis(min, max);
    return dis(random_gen);
}

inline double randReal(double min = 0.0, double max = 1.0) // [min, max)
{
    std::uniform_real_distribution<> dis(min, max);
    return dis(random_gen);
}

////////////////////////////////////////////////////////////////////////////////

std::string strip(std::string& s);
std::string stripImage(std::string& s);
std::string stripFace(std::string& s);


class simple_str
{
private:
    int16_t _len = 0;
    char* _data = NULL;

public:
    int16_t length() const { return _len; }
    const char* c_str() const { return _data; }
    std::string operator()()
    {
        return std::string(_data);
    }

    simple_str() {}
    simple_str(int16_t len, const char* str) : _len(len)
    {
        _data = new char[_len];
        memcpy_s(_data, _len, str, _len);
        _data[_len - 1] = 0;
    }
    simple_str(const char* str) : simple_str(int16_t(strlen(str)), str) {}
    simple_str(const std::string& str) : simple_str(int16_t(str.length()), str.c_str()) {}
    ~simple_str() { if (_data) delete _data; }
    simple_str(const simple_str& str) : simple_str(str.length(), str.c_str()) {}
    simple_str& operator=(const simple_str& str)
    {
        if (_data) delete _data;
        _len = str.length();
        _data = new char[_len];
        memcpy_s(_data, _len, str.c_str(), _len);
        _data[_len - 1] = 0;
        return *this;
    }
    operator std::string() const { return std::string(_data); }
};

/*
Ⱥ��Ա��Ϣ
��**CQ_getGroupMemberInfoV2**���ص���Ϣ
ǰ8���ֽڣ���һ��Int64_t���ȣ�QQȺ�ţ�
������8���ֽڣ���һ��Int64_t���ȣ�QQ�ţ�
������2���ֽڣ���һ��short���ȣ��ǳƳ��ȣ�
�������ǳƳ��ȸ��ֽڣ��ǳ��ı���
������2���ֽڣ���һ��short���ȣ�Ⱥ��Ƭ���ȣ�
������Ⱥ��Ƭ���ȸ��ֽڣ�Ⱥ��Ƭ�ı���
������4���ֽڣ���һ��int���ȣ��Ա�0��1Ů��
������4���ֽڣ���һ��int���ȣ����䣬QQ�ﲻ��ֱ���޸����䣬�Գ�����Ϊ׼��
������2���ֽڣ���һ��short���ȣ��������ȣ�
�������������ȸ��ֽڣ������ı���
������4���ֽڣ���һ��int���ȣ���Ⱥʱ�����
������4���ֽڣ���һ��int���ȣ������ʱ�����
������2���ֽڣ���һ��short���ȣ�Ⱥ�ȼ����ȣ�
������Ⱥ�ȼ����ȸ��ֽڣ�Ⱥ�ȼ��ı���
������4���ֽڣ���һ��int���ȣ�����Ȩ�ޣ�1��Ա��2����Ա��3Ⱥ����
������4���ֽڣ���һ��int���ȣ�0����֪����ʲô�������ǲ�����¼��Ա��
������2���ֽڣ���һ��short���ȣ�ר��ͷ�γ��ȣ�
������ר��ͷ�γ��ȳ��ȸ��ֽڣ�ר��ͷ�γ����ı���
������4���ֽڣ���һ��int���ȣ�ר��ͷ�ι���ʱ�����
������4���ֽڣ���һ��int���ȣ������޸���Ƭ��1�����²�0�ǲ�����
*/
struct GroupMemberInfo
{
    int64_t group;
    int64_t qqid;
    simple_str nick;
    simple_str card;
    int32_t gender;
    int32_t age;
    simple_str area;
    int32_t joinTime;
    int32_t speakTime;
    simple_str level;
    int32_t permission;
    int32_t dummy1;
    simple_str title;
    int32_t titleExpireTime;
    int32_t canModifyCard;

    GroupMemberInfo(const char* base64_decoded);
};

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
inline uint16_t ntohs(uint16_t netshort)
{
    return
        ((netshort & 0x00FF) << 8) +
        ((netshort & 0xFF00) >> 8);
}
inline uint32_t ntohl(uint32_t netlong)
{
    return
        ((netlong & 0x000000FF) << 24) +
        ((netlong & 0x0000FF00) << 8) +
        ((netlong & 0x00FF0000) >> 8) +
        ((netlong & 0xFF000000) >> 24);
}
inline uint64_t ntohll(uint64_t netllong)
{
    return (uint64_t(ntohl(uint32_t(netllong & 0xFFFFFFFF))) << 32) + 
        (uint64_t(ntohl(uint32_t((netllong >> 32) & 0xFFFFFFFF))));
}

//card: 8+8+2+?+2+?
std::string getCardFromGroupInfoV2(const char* base64_decoded);
std::string getCard(int64_t group, int64_t qq);

// 1��Ա��2����Ա��3Ⱥ��
int getPermissionFromGroupInfoV2(const char* base64_decoded);

inline std::string CQ_At(int64_t qq)
{
    std::stringstream ss;
    ss << "[CQ:at,qq=" << qq << "]";
    return ss.str();
}

inline const int TIMEZONE_HR = 8;
inline const int TIMEZONE_MIN = 0;
inline std::tm getLocalTime(int timezone_hr, int timezone_min, time_t offset = 0)
{
    auto t = time(nullptr) + offset;
    t += timezone_hr * 60 * 60 + timezone_min * 60;

    std::tm tm;
    gmtime_s(&tm, &t);
    return tm;
}

inline void broadcastMsg(const char* msg)
{
    CQ_sendGroupMsg(ac, 479733965, msg);
    CQ_sendGroupMsg(ac, 391406854, msg);
}

////////////////////////////////////////////////////////////////////////////////

inline const std::string EMOJI_HORSE    = "[CQ:emoji,id=128052]";
inline const std::string EMOJI_HAMMER   = "[CQ:emoji,id=128296]";
inline const std::string EMOJI_DOWN     = "[CQ:emoji,id=11015]";
inline const std::string EMOJI_NONE     = "[CQ:emoji,id=127514]";
inline const std::string EMOJI_HORN     = "[CQ:emoji,id=128227]";