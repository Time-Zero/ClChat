#pragma once

/*
定义的属于server和client的公共文件
*/

enum EnMsgType{
    LOGIN_MSG = 1,          // 登录消息
    LOGIN_MSG_ACK,
    REG_MSG,            // 注册消息
    REG_MSG_ACK         // 注册相应消息
};