#pragma once
#define CQAPPID "com.yaasdf.bigbedbot" //���޸�AppID������� http://d.cqp.me/Pro/����/������Ϣ
#define CQAPPINFO CQAPIVERTEXT "," CQAPPID

enum enumCQBOOL
{
    FALSE,
    TRUE
};

inline bool enabled = false;

class SQLite;
void timedCommit(SQLite& db);

extern int64_t QQME;
extern time_t banTime_me;

#include "utils.h"

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

class GroupMemberInfo
{
public:
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

    GroupMemberInfo() {}
    GroupMemberInfo(const char* base64_decoded);
};

//card: 8+8+2+?+2+?
std::string getCardFromGroupInfoV2(const char* base64_decoded);
std::string getCard(int64_t group, int64_t qq);

// 1��Ա��2����Ա��3Ⱥ��
int getPermissionFromGroupInfoV2(const char* base64_decoded);

inline std::string CQ_At(int64_t qq)
{
    using namespace std::string_literals;
    return "[CQ:at,qq="s + std::to_string(qq) + "]"s;
}
