// ================== 佛祖保佑，永无BUG ==================
//                        _oo0oo_
//                       o8888888o
//                       88" . "88
//                       (| -_- |)
//                       0\  =  /0
//                     ___/`---'\___
//                   .' \\|     |// '.
//                  / \\|||  :  |||// \
//                 / _||||| -:- |||||- \
//                |   | \\\  -  /// |   |
//                | \_|  ''\---/''  |_/ |
//                \  .-\__  '-'  ___/-. /
//              ___'. .'  /--.--\  `. .'___
//           ."" '<  `.___\_<|>_/___.' >' "".
//          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//          \  \ `_.   \_ __\ /__ _/   .-` /  /
//      =====`-.____`.___ \_____/___.-`___.-'=====
//                        `=---='
//
//      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//             佛祖保佑     永无BUG     永不修改
// ===================================================

// 放到每个文件开头，佛祖会保佑你的代码
#pragma once

#ifndef PLAYER_H
#define PLAYER_H

#include <graphics.h>
#include<iostream>
#include <mmsystem.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <windows.h>  // 添加这个用于MessageBox
#include <stdlib.h>
#include <stdio.h>
#include <io.h>      // 用于 _findfirst, _findnext
#include <direct.h>  // 用于 _mkdir
#include <stdint.h>   //提供储存指针的符号整数

// 播放模式枚举[已查]
enum PlayMode {
    PLAY_MODE_NORMAL = 0,    // 顺序播放
    PLAY_MODE_REPEAT_ONE,    // 单曲循环
    PLAY_MODE_REPEAT_ALL,    // 列表循环
    PLAY_MODE_SHUFFLE        // 随机播放
};

// 按钮结构体[已查]
struct Button {
    int x, y, w, h;
    unsigned long curColor;
    unsigned long oldColor;   //实现按键变色
    char* str;
    int id;  // 按钮ID
};

// 歌曲信息结构体[后期实现下双指针][已查]
struct Song {
    char* name;        // 歌曲名
    char* path;        // 文件路径
    int duration;      // 时长（秒）
    int favorite;      // 是否收藏（1=收藏，0=不收藏）
    struct Song* next; // 下一首歌
};

// 播放列表结构体[已查]
struct Playlist {
    struct Song* head;      // 链表头
    struct Song* current;   // 当前播放的歌曲
    int totalSongs;         // 总歌曲数
    int visibleCount;       // 可视歌曲数量
    int scrollOffset;       // 滚动偏移量
    enum PlayMode playMode; // 播放模式
};

// 按钮相关函数
struct Button* creatButton(int x, int y, int w, int h, unsigned long curColor, const char* str, int id);
void Show(struct Button* pButton);
int InButton(struct Button* pButton, ExMessage msg);
void SetTextButton(struct Button* pButton, const char* str);
void DestroyButton(struct Button* pButton);

// 播放列表相关函数
void initPlaylist(struct Playlist* playlist);
void addSong(struct Playlist* playlist, const char* name, const char* path);
void removeSong(struct Playlist* playlist, int index);
void removeAllSongs(struct Playlist* playlist);
void freePlaylist(struct Playlist* playlist);
void loadDefaultPlaylist(struct Playlist* playlist);
void savePlaylistToFile(struct Playlist* playlist, const char* filename);
void loadPlaylistFromFile(struct Playlist* playlist, const char* filename);
struct Song* getSongAt(struct Playlist* playlist, int index);
int countFavorites(struct Playlist* playlist);
struct Song* getFavoriteSongAt(struct Playlist* playlist, int favIndex);
int getSongFavoriteIndex(struct Playlist* playlist, struct Song* song);

// 音乐控制函数
void playCurrentSong(struct Playlist* playlist);
void playNextSong(struct Playlist* playlist);
void playPrevSong(struct Playlist* playlist);
void stopCurrentSong();
void changePlayMode(struct Playlist* playlist);

// 界面绘制函数
void drawSongList(struct Playlist* playlist, int showOnlyFavorites, int startX, int startY);
void drawCurrentSongInfo(struct Playlist* playlist, int x, int y);
void drawPlayModeInfo(struct Playlist* playlist, int x, int y);

// 交互函数
int handleSongListClick(struct Playlist* playlist, int showOnlyFavorites, ExMessage msg, int startX, int startY);
void scrollPlaylist(struct Playlist* playlist, int direction);
void scrollPlaylistFiltered(struct Playlist* playlist, int showOnlyFavorites, int direction);

//获取文件夹歌曲[已查]
void scanMusicFolder(struct Playlist* playlist, const char* folderPath);

#endif