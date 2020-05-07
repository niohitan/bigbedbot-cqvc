#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <iostream>
#include "dbconn.h"
namespace eat {

inline SQLite db("eat.db", "eat");

enum class commands: size_t {
    ��ʲô,
    ��ʲô,
    ��ʲô,
    ��ʲôʮ��,
    �Ӳ�,
	ɾ��,
	������,
	ɾ����,
    �˵�,
    ɾ��,
};

inline std::map<std::string, commands> commands_str
{
    {"��ʲô", commands::��ʲô},
    {"��ʲô", commands::��ʲô},
    {"��ʲô", commands::��ʲô},
    {"��ʲôʮ��", commands::��ʲôʮ��},
    {"�Ӳ�", commands::�Ӳ�},
	{"����", commands::ɾ��},
	{"ɾ��", commands::ɾ��},
	{"������", commands::������},
	{"ɾ����", commands::ɾ����},
    {"�˵�", commands::�˵�},
    //{"drop", commands::ɾ��},
};

typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
//typedef std::string(*callback)(::int64_t, ::int64_t, std::vector<std::string>);
struct command
{
    commands c = (commands)0;
    std::vector<std::string> args;
    callback func = nullptr;
};

command msgDispatcher(const char* msg);

////////////////////////////////////////////////////////////////////////////////

struct food
{
    std::string name;
    enum {ANONYMOUS, NAME, QQ} offererType;
    struct { std::string name; int64_t qq; } offerer;
    std::string to_string(int64_t group = 0);
};
//inline std::vector<food> foodList;
void foodCreateTable();
//void foodLoadListFromDb();

struct drink
{
    std::string name;
	int64_t qq;
	int64_t group;
};
inline std::vector<drink> drinkList;
void drinkCreateTable();

void updateSteamGameList();
    
}
