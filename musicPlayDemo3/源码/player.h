#pragma once
#ifndef PLAYER_H
#define PLAYER_H

// === 配置文件路径 ===[新增]设计默认和用户双数据源
#define DEFAULT_PLAYLIST_FILE "default_playlist.txt"
#define USER_PLAYLIST_FILE "user_playlist.txt"
#define DEFAULT_LYRIC_FOLDER "./source/lyrics/default/"
#define USER_LYRIC_FOLDER "./source/lyrics/user/"
#define DEFAULT_MUSIC_FOLDER "./source/music/default/"
#define USER_MUSIC_FOLDER "./source/music/user/"
#define LYRIC_FILE_EXTENSION ".lrc"

// === 系统头文件 ===
#include <graphics.h>
#include <iostream>
#include <mmsystem.h>
#include <malloc.h>   
#include <string.h>
#include <time.h>
#include <windows.h>       //引入WindowsAPI
#include <stdlib.h>
#include <stdio.h>
#include <io.h>            //文件操作
#include <direct.h>
#include <stdint.h>
#include <math.h>  
#include <conio.h>

// === 枚举定义 ===
enum PlayMode {
    PLAY_MODE_NORMAL = 0,    // 顺序播放
    PLAY_MODE_REPEAT_ONE,    // 单曲循环
    PLAY_MODE_REPEAT_ALL,    // 列表循环
    PLAY_MODE_SHUFFLE        // 随机播放
};

//[新增]歌词源[自动实现2]
enum LyricSource {
    LYRIC_SOURCE_DEFAULT = 0,    // 从默认文件夹加载
    LYRIC_SOURCE_USER = 1,       // 从用户文件夹加载
    LYRIC_SOURCE_AUTO = 2        // 自动选择（先用户后默认）
};

//[新增]音乐源[2]
enum MusicSource {
    MUSIC_SOURCE_DEFAULT = 0,    // 从默认文件夹加载
    MUSIC_SOURCE_USER = 1,       // 从用户文件夹加载
    MUSIC_SOURCE_AUTO = 2        // 自动选择（先用户后默认）
};

// === 结构体定义 ===
//[新增]歌词行
struct LyricLine {
    int time_ms;            // 时间(毫秒)
    char text[256];         // 歌词文本
    struct LyricLine* next; // 下一行
};

//[新增]整首歌歌词
struct Lyric {
    struct LyricLine* head;   // 歌词行链表头
    int total_lines;          // 总行数
    int current_line;         // 当前行索引
    int visible_count;        // 可视行数
    int scroll_offset;        // 滚动偏移
};

//[新增]歌词数据+路径
struct Song {
    char* name;        // 歌曲名
    char* path;        // 文件路径
    int duration;      // 时长（秒）
    int favorite;      // 是否收藏（1=收藏，0=不收藏）
    struct Song* next; // 下一首歌
    struct Lyric* lyric; // 歌词数据
    char* lyric_path;  // 歌词文件路径
    int visible;
};

//[新增]设计音乐源,歌词源
struct Playlist {
    struct Song* head;
    struct Song* current;
    int totalSongs;
    int visibleSongs;      // 当前可见歌曲数量（根据音乐源过滤后）
    int displayRows;       // 歌单显示行数（固定值，如15）
    int scrollOffset;
    enum PlayMode playMode;
    int volume;
    enum LyricSource lyricSource;
    enum MusicSource musicSource;
};

//[新增]圆形按钮设计
struct Button {
    int x, y, w, h;
    unsigned long curColor;
    unsigned long oldColor;
    char* str;
    int id;           // 按钮ID
    int is_circle;    // 判断圆形按钮
    int radius;       // 设计圆形半径
};

// === 函数声明 ===

// 按钮相关函数
struct Button* creatButton(int x, int y, int w, int h, unsigned long curColor, const char* str, int id);
struct Button* createCircleButton(int x, int y, int radius, unsigned long curColor, const char* str, int id);
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
struct Song* getSongAt(struct Playlist* playlist, int index);
int countFavorites(struct Playlist* playlist);

// 文件管理函数[新增设计两套源]
void savePlaylistToFile(struct Playlist* playlist, const char* filename);     //[封装]保存歌列表到文件
void loadPlaylistFromFile(struct Playlist* playlist, const char* filename);   //[封装]从文件加载歌单
void saveUserPlaylist(struct Playlist* playlist);
void loadUserPlaylist(struct Playlist* playlist);
void loadDefaultPlaylist(struct Playlist* playlist);
void loadDefaultPlaylistFromFile(struct Playlist* playlist);
void scanMusicFolder(struct Playlist* playlist, const char* folderPath);
void scanMusicFolders(struct Playlist* playlist);
char* buildMusicPath(const char* songName, enum MusicSource source);

// 音乐控制函数
void playCurrentSong(struct Playlist* playlist);
void playNextSong(struct Playlist* playlist);
void playPrevSong(struct Playlist* playlist);
void stopCurrentSong();
void changePlayMode(struct Playlist* playlist);
struct Song* getRandomSong(struct Playlist* playlist);

// 歌词相关函数
struct Lyric* loadLyricFromFile(const char* filename);
void freeLyric(struct Lyric* lyric);
void loadLyricForSong(struct Playlist* playlist, struct Song* song);
void reloadLyricsForCurrentSong(struct Playlist* playlist);
void drawLyric(struct Playlist* playlist, int x, int y, int width, int height);
void updateLyricPosition(struct Playlist* playlist);
void scrollLyric(struct Lyric* lyric, int direction);
void setLyricSource(struct Playlist* playlist, enum LyricSource source);
enum LyricSource getLyricSourceForSong(struct Playlist* playlist, struct Song* song);

// 音乐源相关函数
void setMusicSource(struct Playlist* playlist, enum MusicSource source);
enum MusicSource getMusicSourceForSong(struct Playlist* playlist, struct Song* song);

// 进度条相关函数
int getCurrentPlayPosition();
void drawProgressBar(int x, int y, int width, int height, int progress);
int handleProgressBarClick(ExMessage msg, int x, int y, int width, int height);

// 音量相关函数
void drawVolumeBar(struct Playlist* playlist, int x, int y, int width, int height);
int handleVolumeBarClick(struct Playlist* playlist, ExMessage msg, int x, int y, int width, int height);

// 界面绘制函数
void drawSongList(struct Playlist* playlist, int showOnlyFavorites, int startX, int startY);
void drawCurrentSongInfo(struct Playlist* playlist, int x, int y);
void drawPlayModeInfo(struct Playlist* playlist, int x, int y);

// 交互函数
int handleSongListClick(struct Playlist* playlist, int showOnlyFavorites, ExMessage msg, int startX, int startY);
void scrollPlaylist(struct Playlist* playlist, int direction);
void scrollPlaylistFiltered(struct Playlist* playlist, int showOnlyFavorites, int direction);

// 开场视频函数
int playIntroVideo();
extern struct Playlist* g_playlist;  // 全局播放列表指针

#endif 