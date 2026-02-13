  #define _CRT_SECURE_NO_WARNINGS
#include "player.h"

#pragma comment(lib, "winmm.lib")

// === 工具函数 ===
//[1]修改后的获取喜欢的歌单函数
int countVisibleFavorites(struct Playlist* playlist) {
    if (!playlist || !playlist->head) return 0;

    int count = 0;
    struct Song* current = playlist->head;

    while (current) {
        if (current->visible && current->favorite) {
            count++;
        }
        current = current->next;
    }
    return count;
}
//[新增]获取歌曲文件的相对路径[通过信息源类型选取文件夹][1]
char* buildMusicPath(const char* songName, enum MusicSource source) {
    char* fullPath = (char*)malloc(512);
    if (!fullPath) return NULL;

    char nameWithoutExt[256];
    strcpy(nameWithoutExt, songName);
    char* dot = strrchr(nameWithoutExt, '.');
    if (dot) *dot = '\0';

    const char* folder = NULL;
    if (source == MUSIC_SOURCE_DEFAULT) {
        folder = DEFAULT_MUSIC_FOLDER;
    }
    else if (source == MUSIC_SOURCE_USER) {
        folder = USER_MUSIC_FOLDER;
    }

    if (folder) {
        sprintf(fullPath, "%s%s", folder, songName);
    }
    else {
        strcpy(fullPath, songName);
    }
    return fullPath;
}

// === 随机播放函数 ===[1]
struct Song* getRandomSong(struct Playlist* playlist) {
    if (!playlist || !playlist->head || playlist->totalSongs == 0) {
        return NULL;
    }

    if (playlist->totalSongs == 1) {
        return playlist->head;
    }

    int randomIndex;
    struct Song* randomSong;
    do {
        randomIndex = rand() % playlist->totalSongs;
        randomSong = getSongAt(playlist, randomIndex);
    } while (randomSong == playlist->current && playlist->totalSongs > 1);

    return randomSong;
}

// === 文件管理函数 ===
//保存到用户歌单[2]
void saveUserPlaylist(struct Playlist* playlist) {
    savePlaylistToFile(playlist, USER_PLAYLIST_FILE);
}
//[2]
void loadUserPlaylist(struct Playlist* playlist) { 
    loadPlaylistFromFile(playlist, USER_PLAYLIST_FILE);
}  
//[2]
void loadDefaultPlaylistFromFile(struct Playlist* playlist) {
    loadPlaylistFromFile(playlist, DEFAULT_PLAYLIST_FILE);
}

// === 歌词相关函数 ===[1]
//[新增]从目标文件获取歌词[封装]
struct Lyric* loadLyricFromFile(const char* filename) { 
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    struct Lyric* lyric = (struct Lyric*)malloc(sizeof(struct Lyric));
    if (!lyric) {
        fclose(file);
        return NULL;
    }

    lyric->head = NULL;
    lyric->total_lines = 0;
    lyric->current_line = 0;
    lyric->visible_count = 7;
    lyric->scroll_offset = 0;

    char line[512];
    struct LyricLine* tail = NULL;

    while (fgets(line, sizeof(line), file)) {
        // 去除换行符和回车符
        line[strcspn(line, "\r\n")] = 0;

        // 跳过空行
        if (strlen(line) == 0) continue;

        // 解析时间戳
        if (line[0] == '[') {
            int minutes = 0, seconds = 0, milliseconds = 0;
            int parsed = 0;

            // 尝试解析 [mm:ss.xx] 格式
            char* dot = strchr(line, '.');
            if (dot) {
                parsed = sscanf(line, "[%d:%d.%d]", &minutes, &seconds, &milliseconds);
            }
            else {
                parsed = sscanf(line, "[%d:%d]", &minutes, &seconds);
            }

            if (parsed >= 2) {
                struct LyricLine* newLine = (struct LyricLine*)malloc(sizeof(struct LyricLine));
                if (!newLine) continue;

                // 计算毫秒时间
                newLine->time_ms = minutes * 60000 + seconds * 1000 + milliseconds;

                // 提取歌词文本
                char* text_start = strchr(line, ']');
                if (text_start) {
                    text_start++; 
                    while (*text_start == ' ') text_start++;

                    if (strlen(text_start) > 0) {
                        strcpy(newLine->text, text_start);
                    }
                    else {
                        strcpy(newLine->text, "|");
                    }
                }
                else {
                    strcpy(newLine->text, "|");
                }

                newLine->next = NULL;

                // 按时间排序插入
                if (!lyric->head) {
                    lyric->head = newLine;
                    tail = newLine;
                }
                else {
                    // 简单尾插，后续可以优化为排序插入
                    tail->next = newLine;
                    tail = newLine;
                }

                lyric->total_lines++;
            }
        }
    }

    fclose(file);

    // 如果没有解析到任何歌词，释放内存返回NULL
    if (lyric->total_lines == 0) {
        free(lyric);
        return NULL;
    }

    return lyric;
}
void freeLyric(struct Lyric* lyric) {
    if (!lyric) return;

    struct LyricLine* current = lyric->head;
    while (current) {
        struct LyricLine* next = current->next;
        free(current);
        current = next;
    }
    free(lyric);
}
//[2]按歌词源安排实现对应文件夹查询歌词
void loadLyricForSong(struct Playlist* playlist, struct Song* song) {
    if (!song) return;

    if (song->lyric) {
        freeLyric(song->lyric);
        song->lyric = NULL;
    }

    char songName[256];
    strcpy(songName, song->name);
    char* dot = strrchr(songName, '.');
    if (dot) *dot = '\0';

    char lyricPath[512];
    struct Lyric* lyric = NULL;

    enum LyricSource source = playlist->lyricSource;

    if (source == LYRIC_SOURCE_AUTO) {   //自动
        sprintf(lyricPath, "%s%s.lrc", USER_LYRIC_FOLDER, songName);
        lyric = loadLyricFromFile(lyricPath);

        if (!lyric) {       
            sprintf(lyricPath, "%s%s.lrc", DEFAULT_LYRIC_FOLDER, songName);
            lyric = loadLyricFromFile(lyricPath);
        }
    }
    else if (source == LYRIC_SOURCE_USER) {  //用户
        sprintf(lyricPath, "%s%s.lrc", USER_LYRIC_FOLDER, songName);
        lyric = loadLyricFromFile(lyricPath);
    }
    else if (source == LYRIC_SOURCE_DEFAULT) {  //默认
        sprintf(lyricPath, "%s%s.lrc", DEFAULT_LYRIC_FOLDER, songName);
        lyric = loadLyricFromFile(lyricPath);
    }

    if (lyric) {
        song->lyric = lyric;
        if (song->lyric_path) {
            free(song->lyric_path);
        }
        song->lyric_path = (char*)malloc(strlen(lyricPath) + 1);
        if (song->lyric_path) {
            strcpy(song->lyric_path, lyricPath);
        }
    }
    else {
        return; 
    }
}
//[2]重新加载歌词[1]
void reloadLyricsForCurrentSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current) return;
    loadLyricForSong(playlist, playlist->current);
}
//设置播放列表歌词来源的设置接口[1]
void setLyricSource(struct Playlist* playlist, enum LyricSource source) {
    if (!playlist) return;
    playlist->lyricSource = source;
}


//[新增]绘制歌词显示 + 进度条[1]
void drawLyric(struct Playlist* playlist, int x, int y, int width, int height) {
    // 绘制歌词区域背景 - 半透明深色
    setfillcolor(RGB(25, 25, 35));
    solidrectangle(x, y, x + width, y + height);

    // 绘制发光边框
    setlinecolor(RGB(100, 150, 255));
    setlinestyle(PS_SOLID, 3);
    rectangle(x, y, x + width, y + height);

    // 内边框
    setlinecolor(RGB(60, 80, 120));
    setlinestyle(PS_SOLID, 1);
    rectangle(x + 2, y + 2, x + width - 2, y + height - 2);

    // 如果没有歌词
    if (!playlist || !playlist->current || !playlist->current->lyric ||
        playlist->current->lyric->total_lines == 0) {

        settextstyle(28, 0, "微软雅黑");
        settextcolor(RGB(180, 180, 220));
        setbkmode(TRANSPARENT);

        const char* msg = "*暂无歌词*";
        int textW = textwidth(msg);
        int textH = textheight(msg);
        outtextxy(x + (width - textW) / 2, y + (height - textH) / 2, msg);
        return;
    }

    struct Lyric* lyric = playlist->current->lyric;

    int lineHeight = 48;      
    int startY = y + 35;      
    int maxLines = 7;         

    // 获取当前播放时间
    char positionStr[64] = "0";
    mciSendString("status mymusic position", positionStr, sizeof(positionStr), NULL);
    int currentTime_ms = atoi(positionStr);

    // 更新当前行
    struct LyricLine* currentLine = lyric->head;
    int lineIndex = 0;
    int currentLineIndex = 0;

    while (currentLine) {
        if (currentTime_ms >= currentLine->time_ms) {
            currentLineIndex = lineIndex;
        }
        currentLine = currentLine->next;
        lineIndex++;
    }
    lyric->current_line = currentLineIndex;

    // 自动滚动 - 当前行居中
    int targetScroll = lyric->current_line - maxLines / 2;
    if (targetScroll < 0) targetScroll = 0;
    if (targetScroll > lyric->total_lines - maxLines) {
        targetScroll = lyric->total_lines - maxLines;
    }
    lyric->scroll_offset = targetScroll;

    // 定位到滚动位置
    currentLine = lyric->head;
    for (int i = 0; i < lyric->scroll_offset && currentLine; i++) {
        currentLine = currentLine->next;
    }

    // 显示歌词
    int displayIndex = 0;
    lineIndex = lyric->scroll_offset;

    while (currentLine && displayIndex < maxLines) {
        int currentY = startY + displayIndex * lineHeight;

        // === 更大字号设置 ===
        if (lineIndex == lyric->current_line) {
            // 当前行：超大字体，亮色，高亮背景
            settextstyle(32, 0, "微软雅黑");     // 32号字体
            settextcolor(RGB(255, 255, 120));    // 亮黄色

            // 高亮背景 - 半透明发光效果
            setfillcolor(RGB(70, 100, 180)); // 半透明蓝
            solidrectangle(x + 8, currentY - 8,
                x + width - 8, currentY + lineHeight - 12);

            // 额外边框
            setlinecolor(RGB(120, 180, 255));
            setlinestyle(PS_SOLID, 1);
            rectangle(x + 8, currentY - 8,
                x + width - 8, currentY + lineHeight - 12);
        }
        else {
            // 非当前行：大字体，灰色
            settextstyle(26, 0, "微软雅黑");     // 26号字体
            settextcolor(RGB(160, 160, 180));    // 浅灰色
        }

        // 显示歌词文本
        if (strlen(currentLine->text) > 0) {
            int textWidth = textwidth(currentLine->text);
            int textX = x + (width - textWidth) / 2;

            setbkmode(TRANSPARENT);
            outtextxy(textX, currentY, currentLine->text);
        }

        currentLine = currentLine->next;
        lineIndex++;
        displayIndex++;
    }

    // === 更美观的滚动条 ===
    if (lyric->total_lines > maxLines) {
        int scrollBarX = x + width - 16;
        int scrollBarY = y + 20;
        int scrollBarH = height - 40;
        int scrollBarW = 8;

        // 滚动条背景 - 半透明
        setfillcolor(RGB(50, 50, 70));
        solidrectangle(scrollBarX, scrollBarY,
            scrollBarX + scrollBarW, scrollBarY + scrollBarH);

        // 滑块
        float ratio = (float)lyric->scroll_offset / (lyric->total_lines - maxLines);
        int sliderH = scrollBarH * maxLines / lyric->total_lines;
        if (sliderH < 30) sliderH = 30;
        int sliderY = (int)(scrollBarY + (scrollBarH - sliderH) * ratio);

        // 滑块渐变效果
        setfillcolor(RGB(140, 180, 255));
        solidrectangle(scrollBarX, sliderY,
            scrollBarX + scrollBarW, sliderY + sliderH);

        // 滑块边框
        setlinecolor(RGB(200, 220, 255));
        rectangle(scrollBarX, sliderY,
            scrollBarX + scrollBarW, sliderY + sliderH);
    }

    // 绘制顶部装饰条
    setfillcolor(RGB(80, 120, 200));
    solidrectangle(x + 20, y + 5, x + 80, y + 8);
    solidrectangle(x + width - 80, y + 5, x + width - 20, y + 8);

    // 绘制歌词时间指示器
    char timeIndicator[32];
    int minutes = currentTime_ms / 60000;
    int seconds = (currentTime_ms % 60000) / 1000;
    sprintf(timeIndicator, "%02d:%02d", minutes, seconds);

    settextstyle(18, 0, "微软雅黑");
    settextcolor(RGB(180, 200, 255));
    outtextxy(x + width - 80, y + 12, timeIndicator);
}
//[新增]更新当前行[1]
void updateLyricPosition(struct Playlist* playlist) {
    if (!playlist || !playlist->current || !playlist->current->lyric) {
        return;
    }

    struct Lyric* lyric = playlist->current->lyric;
    char positionStr[64];
    mciSendString("status mymusic position", positionStr, sizeof(positionStr), NULL);
    int currentTime_ms = atoi(positionStr);

    struct LyricLine* currentLine = lyric->head;
    int lineIndex = 0;
    int targetLine = -1;

    while (currentLine) {//不断覆盖实现时间戳覆盖
        if (currentTime_ms >= currentLine->time_ms) {
            targetLine = lineIndex;
        }
        else {
            break;
        }
        currentLine = currentLine->next;
        lineIndex++;
    }

    if (targetLine != -1 && targetLine != lyric->current_line) {
        lyric->current_line = targetLine;
        int targetScroll = targetLine - lyric->visible_count / 2;
        if (targetScroll < 0) targetScroll = 0;
        if (targetScroll > lyric->total_lines - lyric->visible_count) {
            targetScroll = lyric->total_lines - lyric->visible_count;
        }
        lyric->scroll_offset = targetScroll;
    }
}
//[新增]计算滚动量传给结构体[1]
void scrollLyric(struct Lyric* lyric, int direction) {
    if (!lyric) return;

    int newOffset = lyric->scroll_offset + direction;
    if (newOffset < 0) newOffset = 0;
    if (newOffset > lyric->total_lines - lyric->visible_count) {
        newOffset = lyric->total_lines - lyric->visible_count;
    }
    if (newOffset < 0) newOffset = 0;
    lyric->scroll_offset = newOffset;
}

// === 音乐源相关函数 ===
// //[1]实现按照音乐源来动态显示歌单中的歌曲
void setMusicSource(struct Playlist* playlist, enum MusicSource source) {
    if (!playlist) return;
    playlist->musicSource = source;

    // 重置可见计数
    int count = 0;
    struct Song* current = playlist->head;

    while (current) {
        if (source == MUSIC_SOURCE_AUTO) {
            current->visible = 1;     //自动模式全部可视
        }
        else if (source == MUSIC_SOURCE_USER) {
            current->visible = (strstr(current->path, USER_MUSIC_FOLDER) != NULL);  //只显示用户的歌单中的歌曲
        }
        else if (source == MUSIC_SOURCE_DEFAULT) {
            current->visible = (strstr(current->path, DEFAULT_MUSIC_FOLDER) != NULL);
        }

        if (current->visible) count++;
        current = current->next;
    }

    // 更新可见数量
    playlist->visibleSongs = count;

    // 如果当前歌曲不可见，切换到第一个可见歌曲[修复小bug]
    if (playlist->current && !playlist->current->visible) {
        struct Song* first = playlist->head;
        while (first && !first->visible) first = first->next;
        playlist->current = first;
        if (first) {
            playCurrentSong(playlist);
        }
    }

    // 重置滚动偏移
    playlist->scrollOffset = 0;
}
//获取音乐源[1]
enum MusicSource getMusicSourceForSong(struct Playlist* playlist, struct Song* song) {
    if (!song || !song->path) return MUSIC_SOURCE_AUTO;

    if (strstr(song->path, USER_MUSIC_FOLDER) != NULL) {
        return MUSIC_SOURCE_USER;
    }
    else if (strstr(song->path, DEFAULT_MUSIC_FOLDER) != NULL) {
        return MUSIC_SOURCE_DEFAULT;
    }
    return MUSIC_SOURCE_AUTO;
}

// === 文件夹扫描函数+路径 ===[封装][1]
void scanMusicFolder(struct Playlist* playlist, const char* folderPath) {
    //检查目标目录
    if (_access(folderPath, 0) == -1) {
        char command[512];
        sprintf(command, "mkdir \"%s\" 2>nul", folderPath);
        system(command);
        if (_access(folderPath, 0) == -1) {
            return;
        }
    }
    // ---------- .MP3文件查找 ----------
    struct _finddata_t fileinfo;
    intptr_t handle;
    char searchPath[512];

    sprintf_s(searchPath, sizeof(searchPath), "%s*.mp3", folderPath);
    handle = _findfirst(searchPath, &fileinfo);

    if (handle == -1) {
        return;
    }

    do {
        if (!(fileinfo.attrib & _A_SUBDIR)) {
            char* filename = fileinfo.name;
            char songName[256];
            strcpy_s(songName, sizeof(songName), filename);
            char fullPath[512];
            sprintf_s(fullPath, sizeof(fullPath), "%s%s", folderPath, filename);

            int alreadyExists = 0;
            struct Song* current = playlist->head;
            while (current) {
                if (strcmp(current->name, songName) == 0) {
                    alreadyExists = 1;
                    break;
                }
                current = current->next;
            }

            if (!alreadyExists) {
                addSong(playlist, songName, fullPath);
            }
        }
    } while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
}
//[新增]合并双源数据实现自动 [2]
void scanMusicFolders(struct Playlist* playlist) {
    // 记录扫描前的歌曲数
    int beforeCount = playlist->totalSongs;
    scanMusicFolder(playlist, USER_MUSIC_FOLDER);    //扫用户库
    int userAdded = playlist->totalSongs - beforeCount;

    int beforeDefault = playlist->totalSongs;
    scanMusicFolder(playlist, DEFAULT_MUSIC_FOLDER); //扫默认库
    int defaultAdded = playlist->totalSongs - beforeDefault;
}
// === 歌单加载 ===[3]
void loadDefaultPlaylist(struct Playlist* playlist) {
    loadUserPlaylist(playlist);      //先加载用户保存的歌单文件[记忆]

    // 用户为空加载默认歌单文件
    if (playlist->totalSongs == 0) {
        loadDefaultPlaylistFromFile(playlist);

        //默认 + 用户全为空 ,扫描文件合并
        if (playlist->totalSongs == 0) {  
            scanMusicFolders(playlist);
            savePlaylistToFile(playlist, DEFAULT_PLAYLIST_FILE);
        }
    }

    // 加载歌词
    struct Song* current = playlist->head;
    while (current) {
        loadLyricForSong(playlist, current);
        current = current->next;
    }
    playlist->current = playlist->head;
    //初始化全部为自动
    playlist->lyricSource = LYRIC_SOURCE_AUTO;
    playlist->musicSource = MUSIC_SOURCE_AUTO;
    playlist->scrollOffset = 0;
    setMusicSource(playlist, MUSIC_SOURCE_AUTO);  //初始化全部显示

    if (playlist->current) {
        playCurrentSong(playlist);
    }
}

// === 按钮相关函数 ===
//[1]
struct Button* creatButton(int x, int y, int w, int h, unsigned long curColor, const char* str, int id) {
    struct Button* pButton = (struct Button*)malloc(sizeof(struct Button));
    if (!pButton) return NULL;

    pButton->x = x;
    pButton->y = y;
    pButton->w = w;
    pButton->h = h;
    pButton->curColor = curColor;
    pButton->oldColor = curColor;
    pButton->id = id;
    pButton->is_circle = 0;
    pButton->radius = 0;

    int length = strlen(str);
    pButton->str = (char*)malloc(length + 1);
    if (pButton->str) {
        strcpy_s(pButton->str, length + 1, str);
    }

    return pButton;
}
//[1]
struct Button* createCircleButton(int x, int y, int radius, unsigned long curColor, const char* str, int id) {
    struct Button* button = (struct Button*)malloc(sizeof(struct Button));
    if (!button) return NULL;

    button->x = x - radius;
    button->y = y - radius;
    button->w = radius * 2;
    button->h = radius * 2;
    button->curColor = curColor;
    button->oldColor = curColor;
    button->id = id;
    button->is_circle = 1;
    button->radius = radius;

    int length = strlen(str);
    button->str = (char*)malloc(length + 1);
    if (button->str) {
        strcpy(button->str, str);
    }

    return button;
}
//[1]
void Show(struct Button* pButton) {
    if (!pButton) return;

    if (pButton->is_circle) {
        setfillcolor(pButton->curColor);
        solidcircle(pButton->x + pButton->radius, pButton->y + pButton->radius, pButton->radius);
        setlinecolor(BLACK);
        circle(pButton->x + pButton->radius, pButton->y + pButton->radius, pButton->radius);

        settextstyle(pButton->radius / 2, 0, "STXINGKA");
        int textw = textwidth(pButton->str);
        int texth = textheight(pButton->str);
        int xx = pButton->x + pButton->radius - textw / 2;
        int yy = pButton->y + pButton->radius - texth / 2;

        setbkmode(TRANSPARENT);
        settextcolor(BLACK);
        outtextxy(xx, yy, pButton->str);
    }
    else {
        setfillcolor(pButton->curColor);
        solidrectangle(pButton->x, pButton->y, pButton->x + pButton->w, pButton->y + pButton->h);

        settextstyle(20, 0, "STXINGKA");
        int textw = textwidth(pButton->str);
        int texth = textheight(pButton->str);
        int xx = pButton->x + (pButton->w - textw) / 2;
        int yy = pButton->y + (pButton->h - texth) / 2;

        setbkmode(TRANSPARENT);
        settextcolor(BLACK);
        outtextxy(xx, yy, pButton->str);
    }
}
//[1]
int InButton(struct Button* pButton, ExMessage msg) {
    if (!pButton) return 0;

    if (pButton->is_circle) {
        int centerX = pButton->x + pButton->radius;
        int centerY = pButton->y + pButton->radius;
        int distance = (int)sqrt(pow(msg.x - centerX, 2) + pow(msg.y - centerY, 2));

        if (distance <= pButton->radius) {
            pButton->curColor = LIGHTBLUE;
            return 1;
        }
        pButton->curColor = pButton->oldColor;
        return 0;
    }
    else {
        if (msg.x >= pButton->x && msg.x <= pButton->x + pButton->w &&
            msg.y >= pButton->y && msg.y <= pButton->y + pButton->h) {
            pButton->curColor = LIGHTBLUE;
            return 1;
        }
        pButton->curColor = pButton->oldColor;
        return 0;
    }
}
//[1]
void SetTextButton(struct Button* pButton, const char* str) {
    if (!pButton) return;

    int length = strlen(str);
    free(pButton->str);
    pButton->str = (char*)malloc(length + 1);
    if (pButton->str) {
        strcpy_s(pButton->str, length + 1, str);
    }
}
//[1]
void DestroyButton(struct Button* pButton) {
    if (pButton) {
        free(pButton->str);
        free(pButton);
    }
}

// === 播放列表相关函数 ===
//[修改+封装]
void initPlaylist(struct Playlist* playlist) {
    playlist->head = NULL;
    playlist->current = NULL;
    playlist->totalSongs = 0;
    playlist->visibleSongs = 0;   
    playlist->displayRows = 15;    
    playlist->scrollOffset = 0;
    playlist->playMode = PLAY_MODE_NORMAL;
    playlist->volume = 80;
    playlist->lyricSource = LYRIC_SOURCE_AUTO;
    playlist->musicSource = MUSIC_SOURCE_AUTO;
}
void removeSong(struct Playlist* playlist, int index) {
    if (index < 0 || index >= playlist->totalSongs || !playlist->head) {
        return;
    }

    struct Song* current = playlist->head;
    struct Song* prev = NULL;

    // 找到要删除的节点
    for (int i = 0; i < index; i++) {
        prev = current;
        current = current->next;
        if (!current) {  // 额外的安全检查
            return;
        }
    }

    if (current == playlist->current) {
        stopCurrentSong();
        if (current->next) {
            playlist->current = current->next;
        }
        else if (prev) {
            playlist->current = prev;
        }
        else {
            playlist->current = NULL;
        }
    }

    if (prev) {
        prev->next = current->next;
    }
    else {
        playlist->head = current->next;
    }

    if (current->lyric) {
        freeLyric(current->lyric);
    }
    if (current->lyric_path) {
        free(current->lyric_path);
    }
    free(current->name);
    free(current->path);
    free(current);

    playlist->totalSongs--;
    if (playlist->totalSongs == 0) {
        playlist->head = NULL;
        playlist->current = NULL;
    }
    savePlaylistToFile(playlist, USER_PLAYLIST_FILE);
}
void addSong(struct Playlist* playlist, const char* name, const char* path) {
    struct Song* newSong = (struct Song*)malloc(sizeof(struct Song));
    if (!newSong) return;

    newSong->name = (char*)malloc(strlen(name) + 1);
    if (!newSong->name) {
        free(newSong);
        return;
    }
    strcpy_s(newSong->name, strlen(name) + 1, name);
    newSong->visible = 1;
    newSong->path = (char*)malloc(strlen(path) + 1);
    if (!newSong->path) {
        free(newSong->name);
        free(newSong);
        return;
    }
    strcpy_s(newSong->path, strlen(path) + 1, path);

    newSong->duration = 0;
    newSong->favorite = 0;
    newSong->next = NULL;
    newSong->lyric = NULL;
    newSong->lyric_path = NULL;

    if (!playlist->head) {
        playlist->head = newSong;
        playlist->current = newSong;
    }
    else {
        struct Song* temp = playlist->head;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = newSong;
    }
    playlist->totalSongs++;
}
void removeAllSongs(struct Playlist* playlist) {
    stopCurrentSong();

    struct Song* current = playlist->head;
    while (current) {
        struct Song* next = current->next;
        if (current->lyric) {
            freeLyric(current->lyric);
        }
        if (current->lyric_path) {
            free(current->lyric_path);
        }
        free(current->name);
        free(current->path);
        free(current);
        current = next;
    }

    playlist->head = NULL;
    playlist->current = NULL;
    playlist->totalSongs = 0;
    playlist->scrollOffset = 0;
    savePlaylistToFile(playlist, USER_PLAYLIST_FILE);  //清空后保存歌单实现记忆性
}
//[封装][1]
void freePlaylist(struct Playlist* playlist) {
    struct Song* current = playlist->head;
    while (current) {
        struct Song* next = current->next;
        if (current->lyric) {
            freeLyric(current->lyric);
        }
        if (current->lyric_path) {
            free(current->lyric_path);
        }
        free(current->name);
        free(current->path);
        free(current);
        current = next;
    }

    playlist->head = NULL;
    playlist->current = NULL;
    playlist->totalSongs = 0;
}
void savePlaylistToFile(struct Playlist* playlist, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        return;
    }

    struct Song* current = playlist->head;
    while (current) {
        fprintf(file, "%s|%s|%d\n", current->name, current->path, current->favorite);
        current = current->next;
    }
    fclose(file);
}
void loadPlaylistFromFile(struct Playlist* playlist, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {       
        line[strcspn(line, "\n")] = 0;              //去除换行符
        //实现切割
        char* name = strtok(line, "|");
        char* path = strtok(NULL, "|");
        char* favStr = strtok(NULL, "|");

        if (name && path) {
            addSong(playlist, name, path);
            if (favStr) {
                struct Song* newSong = playlist->head;
                while (newSong->next) {
                    newSong = newSong->next;
                }
                newSong->favorite = atoi(favStr);
            }
        }
    }
    fclose(file);
}
struct Song* getSongAt(struct Playlist* playlist, int index) {
    if (index < 0 || !playlist) return NULL;

    struct Song* current = playlist->head;
    int visibleIndex = 0;

    while (current) {
        if (current->visible) {
            if (visibleIndex == index) {
                return current;
            }
            visibleIndex++;
        }
        current = current->next;
    }
    return NULL;
}
//[1]
int countFavorites(struct Playlist* playlist) {
    int count = 0;
    struct Song* current = playlist->head;
    while (current) {
        if (current->favorite) count++;
        current = current->next;
    }
    return count;
}  

// === 音乐控制函数 ===
//[修改+封装][1]
void playCurrentSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current) return;

    if (_access(playlist->current->path, 0) == -1) {
        MessageBox(NULL, playlist->current->path, "文件不存在", MB_OK);
        return;
    }
    stopCurrentSong();

    if (!playlist->current->lyric) {
        loadLyricForSong(playlist, playlist->current);
    }

    char cmd[512];
    sprintf(cmd, "open \"%s\" alias mymusic", playlist->current->path);

    MCIERROR error = mciSendString(cmd, NULL, 0, NULL);
    if (error != 0) {
        char errMsg[256];
        mciGetErrorStringA(error, errMsg, sizeof(errMsg));
        return;
    }
    error = mciSendString("play mymusic notify", NULL, 0, NULL);
    if (error != 0) {
        char errMsg[256];
        mciGetErrorStringA(error, errMsg, sizeof(errMsg));
        return;
    }

    char volCmd[64];
    sprintf(volCmd, "setaudio mymusic volume to %d", playlist->volume * 10);
    mciSendString(volCmd, NULL, 0, NULL);
}
//[2]穿透设计
void playNextSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current) {
        return;
    }
    switch (playlist->playMode) {
    case PLAY_MODE_NORMAL:
        if (playlist->current->next) {
            playlist->current = playlist->current->next;
            playCurrentSong(playlist);
        }
        else {
            stopCurrentSong();
            playlist->current = NULL;  // 关键！清空当前播放
        }
        break;

    case PLAY_MODE_REPEAT_ALL:
        if (playlist->current->next) {
            playlist->current = playlist->current->next;
        }
        else {
            playlist->current = playlist->head;
        }
        playCurrentSong(playlist);
        break;

    case PLAY_MODE_REPEAT_ONE:
        playCurrentSong(playlist);
        break;

    case PLAY_MODE_SHUFFLE:
        if (playlist->totalSongs > 1) {
            struct Song* old = playlist->current;
            do {
                playlist->current = getRandomSong(playlist);
            } while (playlist->current == old && playlist->totalSongs > 1);
            playCurrentSong(playlist);
        }
        break;
    }
}   
void playPrevSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current || !playlist->head) return;

    stopCurrentSong();

    switch (playlist->playMode) {
    case PLAY_MODE_NORMAL:
    case PLAY_MODE_REPEAT_ALL:
        if (playlist->current == playlist->head) {
            struct Song* last = playlist->head;
            while (last && last->next) {
                last = last->next;
            }
            playlist->current = last;
        }
        else {
            struct Song* prev = playlist->head;
            while (prev && prev->next != playlist->current) {
                prev = prev->next;
            }
            playlist->current = prev;
        }
        break;

    case PLAY_MODE_REPEAT_ONE:
        break;

    case PLAY_MODE_SHUFFLE:
        playlist->current = getRandomSong(playlist);
        break;
    }

    playCurrentSong(playlist);
}
//[1]
void stopCurrentSong() {
    mciSendString("close mymusic", NULL, 0, NULL);
}
//[1]
void changePlayMode(struct Playlist* playlist) {
    playlist->playMode = (enum PlayMode)((playlist->playMode + 1) % 4);
}

// === 进度条函数 ===
//获取当前播放位置[1]
int getCurrentPlayPosition() {
    char positionStr[64];
    mciSendString("status mymusic position", positionStr, sizeof(positionStr), NULL);
    return atoi(positionStr) / 1000;
}
//[新增]进度条[渐变色填充][1]
void drawProgressBar(int x, int y, int width, int height, int progress) {
    // 背景
    setfillcolor(RGB(50, 50, 70));
    solidrectangle(x, y, x + width, y + height);

    // 进度条
    int progressWidth = (int)(width * (progress / 100.0));
    for (int i = 0; i < progressWidth; i++) {
        int g = 180 + (i * 75 / width);
        setfillcolor(RGB(50, g, 100));
        solidrectangle(x + i, y, x + i + 1, y + height);
    }

    // 边框
    setlinecolor(RGB(120, 120, 160));
    setlinestyle(PS_SOLID, 1);
    rectangle(x, y, x + width, y + height);

    // 当前时间
    int currentSeconds = getCurrentPlayPosition();
    int minutes = currentSeconds / 60;
    int seconds = currentSeconds % 60;

    char timeStr[16];
    sprintf(timeStr, "%02d:%02d", minutes, seconds);

    settextstyle(20, 0, "微软雅黑");
    settextcolor(RGB(180, 220, 255));
    setbkmode(TRANSPARENT);
    outtextxy(x + 8, y - 28, timeStr);

    // 总时长
    char lengthStr[64] = "0";
    mciSendString("status mymusic length", lengthStr, sizeof(lengthStr), NULL);
    int totalSeconds = atoi(lengthStr) / 1000;
    sprintf(timeStr, "%02d:%02d", totalSeconds / 60, totalSeconds % 60);
    outtextxy(x + width - 60, y - 28, timeStr);
}
//[新增]进度条点击拖拽,利用的是进度条设计[1]
int handleProgressBarClick(ExMessage msg, int x, int y, int width, int height) {
    if (msg.message == WM_LBUTTONDOWN) {
        if (msg.x >= x && msg.x <= x + width && msg.y >= y && msg.y <= y + height) {
            float percent = (float)(msg.x - x) / width;
            char lengthStr[64];
            mciSendString("status mymusic length", lengthStr, sizeof(lengthStr), NULL);
            int totalLength_ms = atoi(lengthStr);
            int targetPosition_ms = (int)(totalLength_ms * percent);
           //如果拖到尾部（>=98%），直接触发下一首
            if (percent >= 0.98) {
                playNextSong(g_playlist);
                return 1;
            }
            char cmd[64];
            sprintf(cmd, "seek mymusic to %d", targetPosition_ms);
            mciSendString(cmd, NULL, 0, NULL);
            mciSendString("play mymusic", NULL, 0, NULL);
            return 1;
        }
    }
    return 0;
}

// === 音量函数 ===
//绘制音量条[1]
void drawVolumeBar(struct Playlist* playlist, int x, int y, int width, int height) {
    // 音量条背景
    setfillcolor(RGB(50, 50, 70));
    solidrectangle(x, y, x + width, y + height);

    // 音量进度
    int volumeWidth = (int)(width * (playlist->volume / 100.0));

    // 渐变色音量条
    for (int i = 0; i < volumeWidth; i++) {
        int r = 100 + (i * 155 / width);
        int g = 150 + (i * 105 / width);
        int b = 255;
        setfillcolor(RGB(r, g, b));
        solidrectangle(x + i, y, x + i + 1, y + height);
    }

    // 边框
    setlinecolor(RGB(120, 120, 160));
    setlinestyle(PS_SOLID, 1);
    rectangle(x, y, x + width, y + height);

    // 音量图标 - 更大
    settextstyle(24, 0, "微软雅黑");
    settextcolor(RGB(200, 220, 255));
    setbkmode(TRANSPARENT);

    // 根据音量显示不同图标
    if (playlist->volume == 0) {
        outtextxy(x - 45, y - 5, "0");
    }
    else if (playlist->volume < 30) {
        outtextxy(x - 45, y - 5, "_");
    }
    else if (playlist->volume < 70) {
        outtextxy(x - 45, y - 5, "-");
    }
    else {
        outtextxy(x - 45, y - 5, "=");
    }

    // 音量百分比 - 更大字体
    settextstyle(20, 0, "微软雅黑");
    char volumeStr[16];
    sprintf(volumeStr, "%d%%", playlist->volume);
    outtextxy(x + width + 15, y - 3, volumeStr);
}
//处理音量条点击[1]
int handleVolumeBarClick(struct Playlist* playlist, ExMessage msg, int x, int y, int width, int height) {
    if (msg.message == WM_LBUTTONDOWN) {
        if (msg.x >= x && msg.x <= x + width && msg.y >= y && msg.y <= y + height) {
            float percent = (float)(msg.x - x) / width;
            int newVolume = (int)(percent * 100);
            if (newVolume < 0) newVolume = 0;
            if (newVolume > 100) newVolume = 100;
            playlist->volume = newVolume;

            char cmd[64];
            sprintf(cmd, "setaudio mymusic volume to %d", newVolume * 10);
            mciSendString(cmd, NULL, 0, NULL);
            return 1;
        }
    }
    return 0;
}

// === 界面绘制函数 ===
//绘制歌单列表 + 小标题 [1]
void drawSongList(struct Playlist* playlist, int showOnlyFavorites, int startX, int startY) {
    if (!playlist || !playlist->head) return;
    // 常量定义
    const int rowHeight = 28;
    const int rowSpacing = 2;
    int visibleRows = playlist->displayRows;

    // 计算当前可见歌曲数
    int totalVisible = showOnlyFavorites ? countVisibleFavorites(playlist) : playlist->visibleSongs;
    int hasVisibleSongs = (totalVisible > 0);

    settextstyle(16, 0, "宋体");
    setbkmode(TRANSPARENT);
    settextcolor(YELLOW);
    //标题
    char title[100];
    const char* sourceNames[] = { "默认音乐", "用户音乐", "自动" };
    int favCount = countFavorites(playlist);

    if (showOnlyFavorites) {
        sprintf(title, "收藏歌曲 (共%d首)", favCount);
    }
    else {
        if (playlist->totalSongs > 0 && totalVisible == 0) {
            sprintf(title, "%s - 歌单 (当前无可见歌曲)", sourceNames[playlist->musicSource]);
        }
        else {
            sprintf(title, "%s - 歌单 (%d首)", sourceNames[playlist->musicSource], totalVisible);
        }
    }
    // 绘制标题框
    int titleWidth = textwidth(title);
    int titleBoxLeft = startX - 15;
    int titleBoxRight = startX + titleWidth + 105;
    int titleBoxTop = startY + 10;
    int titleBoxBottom = startY - 27;
    setfillcolor(RGB(40, 40, 60));
    setlinecolor(RGB(100, 100, 150));
    setlinestyle(PS_SOLID, 2);
    solidroundrect(titleBoxLeft, titleBoxTop, titleBoxRight, titleBoxBottom, 8, 8);
    roundrect(titleBoxLeft, titleBoxTop, titleBoxRight, titleBoxBottom, 8, 8);
    int titleX = titleBoxLeft + (titleBoxRight - titleBoxLeft - titleWidth) / 2;
    outtextxy(titleX, startY - 17, title);
    // 列表区域
    int listStartY = startY + 10;
    int listHeight = visibleRows * (rowHeight + rowSpacing) - rowSpacing;
    int scrollBarLeft = startX + 305;
    int scrollBarRight = scrollBarLeft + 8;
    int scrollBarHeight = listHeight;

    // 绘制滚动条背景（只有有可见歌曲时才显示）
    if (hasVisibleSongs) {
        setfillcolor(RGB(70, 70, 90));
        solidrectangle(scrollBarLeft, listStartY, scrollBarRight, listStartY + scrollBarHeight);
    }
    if (hasVisibleSongs && totalVisible > visibleRows) {
        float visibleRatio = (float)visibleRows / totalVisible;
        int sliderHeight = (int)(scrollBarHeight * visibleRatio);
        if (sliderHeight < 20) sliderHeight = 20;
        int maxOffset = totalVisible - visibleRows;
        if (playlist->scrollOffset > maxOffset) {
            playlist->scrollOffset = maxOffset;
        }
        if (playlist->scrollOffset < 0) {
            playlist->scrollOffset = 0;
        }

        float scrollRatio = (float)playlist->scrollOffset / maxOffset;
        int sliderY = listStartY + (int)((scrollBarHeight - sliderHeight) * scrollRatio);

        setfillcolor(RGB(150, 150, 180));
        solidrectangle(scrollBarLeft, sliderY, scrollBarRight, sliderY + sliderHeight);
        setlinecolor(RGB(180, 180, 210));
        rectangle(scrollBarLeft, sliderY, scrollBarRight, sliderY + sliderHeight);
    }

    // 如果没有可见歌曲，直接显示空状态提示并返回
    if (!hasVisibleSongs) {
        int centerX = startX + 150;
        int centerY = listStartY + listHeight / 2;

        settextcolor(LIGHTCYAN);
        if (showOnlyFavorites) {
            outtextxy(centerX - 60, centerY - 20, "没有收藏的歌曲");
            settextcolor(RGB(180, 180, 200));
            outtextxy(centerX - 75, centerY + 5, "点击鼠标中键可以收藏");
        }
        else {
            if (playlist->totalSongs > 0) {
                outtextxy(centerX - 70, centerY - 20, "当前音乐源无歌曲");
                settextcolor(RGB(180, 180, 200));
                outtextxy(centerX - 80, centerY + 5, "请切换音乐源或重新扫描");
            }
            else {
                outtextxy(centerX - 40, centerY - 20, "当前歌单为空");
                settextcolor(RGB(180, 180, 200));
                outtextxy(centerX - 70, centerY + 5, "请添加歌曲或重新扫描");
            }
        }

        // 绘制边框后返回
        setlinecolor(RGB(90, 90, 120));
        setlinestyle(PS_SOLID, 2);
        rectangle(startX - 15, listStartY - 2,
            startX + 302, listStartY + listHeight + 2);
        return;
    }

    // 遍历显示歌曲（有可见歌曲时）
    struct Song* current = playlist->head;
    int y = listStartY;
    int displayIndex = 0;

    int skipCount = 0;
    while (current && skipCount < playlist->scrollOffset) {
        if (current->visible && (!showOnlyFavorites || current->favorite)) {
            skipCount++;
        }
        current = current->next;
    }
    while (current && displayIndex < visibleRows) {
        // 跳过不可见/不收藏的歌曲
        if (!current->visible || (showOnlyFavorites && !current->favorite)) {
            current = current->next;
            continue;
        }

        // 绘制行背景
        int rowTop = y;
        int rowBottom = y + rowHeight;
        int rowLeft = startX - 15;
        int rowRight = startX + 300;

        // 交替颜色
        setfillcolor((displayIndex % 2 == 0) ? RGB(50, 50, 60) : RGB(55, 55, 65));
        solidrectangle(rowLeft, rowTop, rowRight, rowBottom);

        // 边框
        setlinecolor(RGB(70, 70, 85));
        setlinestyle(PS_SOLID, 1);
        rectangle(rowLeft, rowTop, rowRight, rowBottom);

        // 当前播放歌曲高亮
        if (current == playlist->current) {
            setfillcolor(RGB(40, 70, 110));
            setlinecolor(RGB(60, 100, 150));
            solidrectangle(rowLeft, rowTop + 2, rowRight - 2, rowBottom - 2);
            rectangle(rowLeft, rowTop + 2, rowRight - 2, rowBottom - 2);
        }

        // 歌曲文本
        char display[100];
        if (showOnlyFavorites) {
            sprintf(display, "%d.%s %s", displayIndex + 1,
                current->favorite ? "**" : "//", current->name);
        }
        else {
            int index = playlist->scrollOffset + displayIndex + 1;
            sprintf(display, "%d.%s %s", index,
                current->favorite ? "**" : "//", current->name);
        }

        // 颜色
        settextcolor(current->favorite ? LIGHTRED : LIGHTGRAY);

        // 长文本截断
        int maxTextWidth = 240;
        if (textwidth(display) > maxTextWidth) {
            char clipped[100];
            strncpy(clipped, display, 28);
            clipped[28] = '\0';
            strcat(clipped, "...");
            outtextxy(rowLeft + 8, rowTop + 6, clipped);
        }
        else {
            outtextxy(rowLeft + 8, rowTop + 6, display);
        }

        current = current->next;
        y += rowHeight + rowSpacing;
        displayIndex++;
    }

    // 边框
    setlinecolor(RGB(90, 90, 120));
    setlinestyle(PS_SOLID, 2);
    rectangle(startX - 15, listStartY - 2,
        startX + 302, listStartY + listHeight + 2);
}

//[1]165当前播放标题
void drawCurrentSongInfo(struct Playlist* playlist, int x, int y) {
    if (!playlist || !playlist->current) return;

    const int BOX_WIDTH = 400;
    const int BOX_HEIGHT = 45;
    const int TEXT_MAX_CHARS = 25;

    int boxX1 = 165;
    int boxY1 = y;
    int boxX2 = boxX1 + BOX_WIDTH;
    int boxY2 = boxY1 + BOX_HEIGHT;

    setfillcolor(RGB(240, 240, 245));
    setlinecolor(RGB(180, 180, 200));
    setlinestyle(PS_SOLID, 2);

    solidroundrect(165, boxY1, boxX2, boxY2, 8, 8);
    roundrect(165, boxY1, boxX2, boxY2, 8, 8);

    setfillcolor(RGB(0, 180, 0));
    solidcircle(boxX1 + 25, boxY1 + BOX_HEIGHT / 2, 5);

    char displayText[256];
    strncpy(displayText, playlist->current->name, 255);
    displayText[255] = '\0';

    if (strlen(displayText) > TEXT_MAX_CHARS) {
        displayText[TEXT_MAX_CHARS] = '\0';
        strcat(displayText, "...");
    }

    settextstyle(18, 0, "STXINGKA");
    setbkmode(TRANSPARENT);

    int textY = boxY1 + (BOX_HEIGHT - textheight(displayText)) / 2;

    if (playlist->current->favorite) {
        settextcolor(LIGHTRED);
        outtextxy(boxX1 + 45, textY, "** ");
        settextcolor(LIGHTCYAN);
        outtextxy(boxX1 + 70, textY, displayText);
    }
    else {
        settextcolor(BLACK);
        outtextxy(boxX1 + 45, textY, displayText);
    }
}
//[1]165状态 + M 键切换提示
void drawPlayModeInfo(struct Playlist* playlist, int x, int y) {
    settextstyle(16, 0, "宋体");

    const char* modeNames[] = { "顺序播放", "单曲循环", "列表循环", "随机播放" };
    const COLORREF modeColors[] = {
        RGB(0, 160, 70), RGB(60, 100, 200), RGB(0, 140, 140), RGB(160, 0, 160)
    };

    char modeText[50];
    sprintf(modeText, "模式: %s", modeNames[playlist->playMode]);

    int textWidth = textwidth(modeText);
    int textHeight = textheight(modeText);
    setfillcolor(RGB(50, 50, 50));
    setlinecolor(RGB(100, 100, 100));
    setlinestyle(PS_SOLID, 1);

    solidroundrect(165, 95, 373, y + textHeight + 5, 8, 8);
    roundrect(165, 95, 373, y + textHeight + 5, 8, 8);

    setbkmode(TRANSPARENT);
    settextcolor(modeColors[playlist->playMode]);
    outtextxy(x, y, modeText);

    settextstyle(14, 0, "宋体");
    const char* hintText = "[M键切换]";
    int hintWidth = textwidth(hintText);

    setfillcolor(RGB(80, 80, 80));
    setlinecolor(RGB(120, 120, 120));

    solidroundrect(x - 15, y + textHeight + 5, 373, y + textHeight + 25, 5, 5);
    roundrect(x - 15, y + textHeight + 5, 373, y + textHeight + 25, 5, 5);

    settextcolor(RGB(180, 180, 180));
    outtextxy(x, y + textHeight + 10, hintText);
}

// === 交互函数 ===
//[1]
int handleSongListClick(struct Playlist* playlist, int showOnlyFavorites, ExMessage msg, int startX, int startY) {
    if (!playlist || !playlist->head) return 0;
    int listStartY = startY + 15 ;  
    if (msg.x >= startX && msg.x <= startX + 300 &&
        msg.y >= listStartY && msg.y <= listStartY + playlist->displayRows * 28) {
        int itemIndex = (msg.y - listStartY + 1 ) / 28;
        if (itemIndex < 0) itemIndex = 0;
        if (itemIndex >= playlist->displayRows) itemIndex = playlist->displayRows - 1;

        int targetIndex = playlist->scrollOffset + itemIndex;
        struct Song* clickedSong = NULL;
        int originalIndex = -1;
        int totalVisible = showOnlyFavorites ? countVisibleFavorites(playlist) : playlist->visibleSongs;

        if (targetIndex >= 0 && targetIndex < totalVisible) {
            if (showOnlyFavorites) {
                int visibleFavCount = 0;
                struct Song* current = playlist->head;
                while (current) {
                    if (current->visible && current->favorite) {
                        if (visibleFavCount == targetIndex) {
                            clickedSong = current;
                            break;
                        }
                        visibleFavCount++;
                    }
                    current = current->next;
                }
            }
            else {
                clickedSong = getSongAt(playlist, targetIndex);
            }
            if (clickedSong) {
                struct Song* temp = playlist->head;
                originalIndex = 0;
                while (temp && temp != clickedSong) {
                    originalIndex++;
                    temp = temp->next;
                }
            }
        }

        if (clickedSong) {
            if (msg.message == WM_LBUTTONDOWN) {
                playlist->current = clickedSong;
                playCurrentSong(playlist);
                return 1;
            }
            else if (msg.message == WM_RBUTTONDOWN) {
                if (originalIndex >= 0) {
                    removeSong(playlist, originalIndex);
                    return 2;
                }
            }
            else if (msg.message == WM_MBUTTONDOWN) {
                clickedSong->favorite = !clickedSong->favorite;
                savePlaylistToFile(playlist, USER_PLAYLIST_FILE);
                return 3;
            }
        }
    }
    return 0;
}
//[1]
void scrollPlaylist(struct Playlist* playlist, int direction) {
    if (!playlist) return;

    int totalVisible = playlist->visibleSongs;  // 当前可见歌曲数
    int maxOffset = totalVisible - playlist->displayRows;
    if (maxOffset < 0) maxOffset = 0;

    int newOffset = playlist->scrollOffset + direction;
    if (newOffset < 0) newOffset = 0;
    if (newOffset > maxOffset) newOffset = maxOffset;

    playlist->scrollOffset = newOffset;
}
//[2]
void scrollPlaylistFiltered(struct Playlist* playlist, int showOnlyFavorites, int direction) {
    if (!playlist) return;

    if (!showOnlyFavorites) {
        scrollPlaylist(playlist, direction);
        return;
    }

    int favCount = countVisibleFavorites(playlist);
    if (favCount <= playlist->displayRows) {
        playlist->scrollOffset = 0;
        return;
    }

    int newOffset = playlist->scrollOffset + direction;
    if (newOffset < 0) newOffset = 0;
    if (newOffset > favCount - playlist->displayRows) {
        newOffset = favCount - playlist->displayRows;
    }

    playlist->scrollOffset = newOffset;
}

//[新增] === 开场视频函数 ===
// [1]
int playIntroVideo() {
    char videoPath[] = "./source/video/welcome.mp4";

    cleardevice();
    setbkcolor(RGB(30, 40, 60));
    cleardevice();

    settextcolor(LIGHTCYAN);
    settextstyle(32, 0, "STXINGKA");
    outtextxy(400, 300, "正在打开视频...");
    FlushBatchDraw();
    Sleep(1000);

    HINSTANCE hResult = ShellExecute(NULL, "open", videoPath, NULL, NULL, SW_SHOWMAXIMIZED);
    //失败调用直接使用cmd命令行开启
    if ((INT_PTR)hResult <= 32) {
        char sysCmd[512];
        sprintf(sysCmd, "start \"\" \"%s\"", videoPath);
        system(sysCmd);
    }

    cleardevice();
    setbkcolor(RGB(30, 40, 60));
    cleardevice();

    settextcolor(LIGHTCYAN);
    settextstyle(32, 0, "STXINGKA");
    outtextxy(400, 200, "视频正在外部播放器中打开");

    settextcolor(LIGHTGRAY);
    settextstyle(24, 0, "宋体");
    outtextxy(400, 280, "请观看视频...,播放完成后非默认播放器自己关闭");
    outtextxy(400, 320, "点击命令行按下ESC键关闭播放器并继续");

    settextstyle(36, 0, "STXINGKA");
    settextcolor(YELLOW);
    outtextxy(550, 420, "[ESC]");

    settextstyle(36, 0, "STXINGKA");
    settextcolor(YELLOW);
    outtextxy(400, 420, "看完敲任意键到命令框开启音乐之旅");
    FlushBatchDraw();

    int escPressed = 0;
    while (1) {
        if (_kbhit()) {
            int key = _getch();
            if (key == 27) {   //esc启动强制结束
                escPressed = 1;
                cleardevice();
                setbkcolor(RGB(60, 30, 30));
                cleardevice();
                settextcolor(YELLOW);
                settextstyle(32, 0, "STXINGKA");
                outtextxy(400, 300, "正在关闭播放器...");
                FlushBatchDraw();

                system("taskkill /f /im wmplayer.exe >nul 2>nul");  //Windows11默认系统播放器 :wmplayer.exe	Windows Media Player
                system("taskkill /f /im Video.UI.exe >nul 2>nul");  //Windows7默认系统播放器 : Video.UI.exe	Windows 电影与电视

                Sleep(1000);
                break;
            }
            else {          //其他退出循环,显示播放完毕 ; 
                escPressed = 0;
                break;
            }
        }
        Sleep(10);
    }

    cleardevice();
    setbkcolor(RGB(10, 20, 40));
    cleardevice();

    settextcolor(LIGHTCYAN);
    settextstyle(48, 0, "STXINGKA");

    if (escPressed) {
        outtextxy(450, 200, "视频已关闭");
        settextcolor(LIGHTGRAY);
        settextstyle(24, 0, "宋体");
        outtextxy(450, 280, "播放器已被强制关闭");
    }
    else {
        outtextxy(450, 200, "开场视频播放完成");
    }

    int buttonX = 440;
    int buttonY = 350;
    int buttonWidth = 400;
    int buttonHeight = 100;

    setfillcolor(RGB(0, 120, 200));
    setlinecolor(RGB(0, 180, 255));
    setlinestyle(PS_SOLID, 4);

    solidroundrect(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight, 20, 20);
    roundrect(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight, 20, 20);

    settextcolor(WHITE);
    settextstyle(48, 0, "STXINGKA");
    setbkmode(TRANSPARENT);
    outtextxy(buttonX + 45, buttonY + 32, "平a进入妙妙屋");

    settextcolor(LIGHTGRAY);
    settextstyle(24, 0, "宋体");
    outtextxy(500, 500, "点击按钮或按任意键继续");
    FlushBatchDraw();

    ExMessage msg;
    while (1) {
        if (peekmessage(&msg)) {
            if (msg.message == WM_LBUTTONDOWN) {   //鼠标查询
                if (msg.x >= buttonX && msg.x <= buttonX + buttonWidth &&
                    msg.y >= buttonY && msg.y <= buttonY + buttonHeight) {
                    return 1;
                }
            }
            if (msg.message == WM_KEYDOWN) {  //键盘查询
                return 1;
            }
        }
        Sleep(10);
    }
    return 1;
}

// === 自动下一首轮询线程 ===  
// [1]
struct Playlist* g_playlist = NULL;
