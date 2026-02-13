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
#define _CRT_SECURE_NO_WARNINGS
#include "player.h"
#pragma comment(lib, "winmm.lib")

// 配置文件路径
#define PLAYLIST_FILE "playlist.txt"

//统计收藏歌曲数量[循环链表找标记][已查]
int countFavorites(struct Playlist* playlist) {
    int count = 0;
    struct Song* current = playlist->head;
    while (current) {
        if (current->favorite) count++;
        current = current->next;
    }
    return count;
}

// 获取歌曲在收藏列表中的索引[已查]
int getSongFavoriteIndex(struct Playlist* playlist, struct Song* song) {
    if (!song) return -1;
    struct Song* current = playlist->head;
    int favIndex = 0;
    while (current) {
        if (current == song) {
            return favIndex;
        }
        if (current->favorite) {
            favIndex++;
        }
        current = current->next;
    }
    return -1;
}

//通过收藏索引获取收藏歌曲[已查]
struct Song* getFavoriteSongAt(struct Playlist* playlist, int favIndex) {
    if (favIndex < 0) return NULL;
    struct Song* current = playlist->head;
    int favCount = 0;
    while (current) {
        if (current->favorite) {
            if (favCount == favIndex) {
                return current;
            }
            favCount++;
        }
        current = current->next;
    }
    return NULL;
}

// === 按钮相关函数 ===

//创建按钮[已查]
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

    int length = strlen(str);
    pButton->str = (char*)malloc(length + 1);
    if (!pButton->str) {
        free(pButton);
        return NULL;
    }
    strcpy_s(pButton->str, length + 1, str);

    return pButton;
}

//控制显示按钮[已查]
void Show(struct Button* pButton) {
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

//设计鼠标进入框时的状态变化[已查]
int InButton(struct Button* pButton, ExMessage msg) {
    if (msg.x >= pButton->x && msg.x <= pButton->x + pButton->w &&
        msg.y >= pButton->y && msg.y <= pButton->y + pButton->h) {
        pButton->curColor = LIGHTBLUE;
        return 1;
    }
    pButton->curColor = pButton->oldColor;
    return 0;
}

//设计按钮内部文本[已查]
void SetTextButton(struct Button* pButton, const char* str) {
    int length = strlen(str);
    free(pButton->str);
    pButton->str = (char*)malloc(length + 1);
    if (pButton->str) {
        strcpy_s(pButton->str, length + 1, str);
    }
}

//封装删按钮[已查]
void DestroyButton(struct Button* pButton) {
    if (pButton) {
        free(pButton->str);
        free(pButton);
    }
}

// === 播放列表相关函数 ===

//初始化歌曲列表各项参数[后期需要本地处理] [已查]
void initPlaylist(struct Playlist* playlist) {
    playlist->head = NULL;
    playlist->current = NULL;
    playlist->totalSongs = 0;
    playlist->visibleCount = 15;                //定义界面显示限制数量
    playlist->scrollOffset = 0;                 //设置滚动迁移位置
    playlist->playMode = PLAY_MODE_NORMAL;      // 正确使用枚举值

    // 尝试从文件加载歌单
    loadPlaylistFromFile(playlist, PLAYLIST_FILE);

    // 如果文件没有歌单，加载默认歌单[后期补这里分文件管理]
    if (playlist->totalSongs == 0) {
        loadDefaultPlaylist(playlist);
    }
}

//尾插法单链表添加歌曲[已查]
void addSong(struct Playlist* playlist, const char* name, const char* path) {  
    struct Song* newSong = (struct Song*)malloc(sizeof(struct Song));
    if (!newSong) return;

    newSong->name = (char*)malloc(strlen(name) + 1);
    if (!newSong->name) {             //防止内存超限, 堆空间耗尽
        free(newSong);
        return;
    }
    strcpy_s(newSong->name, strlen(name) + 1, name);

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

    if (!playlist->head) {
        playlist->head = newSong;
        playlist->current = newSong;
    }
    else {   //实现尾插
        struct Song* temp = playlist->head;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = newSong;
    }
    playlist->totalSongs++;
}

//删除歌单中的曲目[已查]
void removeSong(struct Playlist* playlist, int index) {
    if (index < 0 || index >= playlist->totalSongs || !playlist->head) {  //判空
        return;
    }

    struct Song* current = playlist->head;
    struct Song* prev = NULL;

    for (int i = 0; i < index; i++) {   //设计双指针指向
        prev = current;
        current = current->next;
    }

    if (current == playlist->current) {  
        stopCurrentSong();  
        if (current->next) {
            playlist->current = current->next;
        }
        else if (prev) {        //最后一首删除返回倒数第二首
            playlist->current = prev;
        }
        else {
            playlist->current = NULL;
        }
    }

    if (prev) {
        prev->next = current->next;       //跳过当前被删文件
    }
    else {
        playlist->head = current->next;
    }

    free(current->name);
    free(current->path);
    free(current);

    playlist->totalSongs--;
    savePlaylistToFile(playlist, PLAYLIST_FILE);
}

//删除当前歌单[已查]
void removeAllSongs(struct Playlist* playlist) {
    stopCurrentSong();

    struct Song* current = playlist->head;
    while (current) {
        struct Song* next = current->next;
        free(current->name);
        free(current->path);
        free(current);
        current = next;
    }

    playlist->head = NULL;
    playlist->current = NULL;
    playlist->totalSongs = 0;
    playlist->scrollOffset = 0;
    savePlaylistToFile(playlist, PLAYLIST_FILE);
}

//释放歌单资源[已查]
void freePlaylist(struct Playlist* playlist) {
    struct Song* current = playlist->head;
    while (current) {
        struct Song* next = current->next;
        free(current->name);
        free(current->path);
        free(current);
        current = next;
    }

    playlist->head = NULL;
    playlist->current = NULL;
    playlist->totalSongs = 0;
}

//加载默认歌曲进入当前的空歌单[已查]
void loadDefaultPlaylist(struct Playlist* playlist) {
    // 先扫描文件夹
    scanMusicFolder(playlist, "./source/music");

    // 如果扫描后还是没有歌曲，添加默认歌曲
    if (playlist->totalSongs == 0) {
        printf("太可怜了，给你吃点好的(●ˇ∀ˇ●)...\n");

        addSong(playlist, "从前说 - 小阿七.mp3", "./source/music/从前说 - 小阿七.mp3");
        addSong(playlist, "如果爱忘了(Live) - 汪苏泷&单依纯.mp3", "./source/music/如果爱忘了(Live) - 汪苏泷&单依纯.mp3");
        addSong(playlist, "说散就散 - 袁娅维TIA RAY.mp3", "./source/music/说散就散 - 袁娅维TIA RAY.mp3");
        addSong(playlist, "天空之外 - 弦子.mp3", "./source/music/天空之外 - 弦子.mp3");
        addSong(playlist, "月牙湾 - F.I.R.飞儿乐团.mp3", "./source/music/月牙湾 - F.I.R.飞儿乐团.mp3");

        // 保存默认歌单到文件
        savePlaylistToFile(playlist, PLAYLIST_FILE);
    }

    playlist->current = playlist->head;
}

//保存当前列表歌曲到歌单[输出在本地可在下次扫时保存][已查]
void savePlaylistToFile(struct Playlist* playlist, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("错误：无法打开本地文件 '%s'::>_<::\n", filename);
        return;
    }

    struct Song* current = playlist->head;
    while (current) {
        fprintf(file, "%s|%s|%d\n", current->name, current->path, current->favorite);  //[输出文本到文件]
        current = current->next;
    }

    fclose(file);
}

//从保存列表歌单找歌曲信息[已查]
void loadPlaylistFromFile(struct Playlist* playlist, const char* filename) {
    FILE* file = fopen(filename, "r");   //只以只读的方式打开文件 ; 
    if (!file) {
        printf("错误：无法打开本地文件 '%s'>_<\n", filename);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;   //去除换行符
        //分割歌曲信息 
        char* name = strtok(line, "|");
        char* path = strtok(NULL, "|");
        char* favStr = strtok(NULL, "|");  //收藏标志

        if (name && path) {
            addSong(playlist, name, path);

            if (favStr) {
                struct Song* needAddSong = playlist->head;
                while (needAddSong->next) {
                    needAddSong = needAddSong->next;
                }
                needAddSong->favorite = atoi(favStr); //atoi字符串->整数 , 标记为1收藏
            }
        }
    }

    fclose(file);

    if (playlist->head) {
        playlist->current = playlist->head;
    }
}

//通过索引获取当前播放列表中的歌曲[已查]
struct Song* getSongAt(struct Playlist* playlist, int index) {
    if (index < 0 || index >= playlist->totalSongs) {
        return NULL;
    }

    struct Song* current = playlist->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }

    return current;
}

// === 音乐控制函数 === 

// 播放当前歌曲[已查]
void playCurrentSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current) return;
    //停止当前播放歌曲 
    stopCurrentSong();

    //MCI命令实现
    char cmd[512];   
    sprintf_s(cmd, sizeof(cmd), "open \"%s\" alias mymusic", playlist->current->path);  //分配别名方便引用
    MCIERROR error = mciSendString(cmd, NULL, 0, NULL);  //简单处理下出错情况 , 无文件时简单说明 
    if (error == 0) {
        error = mciSendString("play mymusic", NULL, 0, NULL);
        if (error != 0) {
            printf("命令失败 ＞_＜\n");
        }
    }
    else {
        printf("打开本地音频文件失败>_<\n");
    }
}

//下一首歌曲/添加模式播放 [后期修复下循环播放的问题][已查]
void playNextSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current) return;

    // 根据播放模式决定下一首歌　调整首歌位置
    switch (playlist->playMode) {
    case PLAY_MODE_NORMAL:
    case PLAY_MODE_REPEAT_ALL:
        if (playlist->current->next) {
            playlist->current = playlist->current->next;
        }
        else {
            if (playlist->playMode == PLAY_MODE_REPEAT_ALL) {
                playlist->current = playlist->head;
            }
        }
        break;

    case PLAY_MODE_REPEAT_ONE:
        break;

    case PLAY_MODE_SHUFFLE:
        // 随机播放
        if (playlist->totalSongs > 1) {
            int randomIndex = rand() % playlist->totalSongs;
            playlist->current = getSongAt(playlist, randomIndex);
        }
        break;
    }

    playCurrentSong(playlist);
}

//上一首歌曲/添加循环 [这里后期可以优化Song实现双链表][已查]
void playPrevSong(struct Playlist* playlist) {
    if (!playlist || !playlist->current || !playlist->head) return;  //参数无效直接返回
    if (playlist->current == playlist->head) {
        //第一首 + 循环 -> 返回最后一首
        if (playlist->playMode == PLAY_MODE_REPEAT_ALL) {
            struct Song* last = playlist->head;
            while (last->next) {
                last = last->next;
            }
            playlist->current = last;
        }
    }
    //非第一首 , 实现上一首
    else {
        struct Song* prev = playlist->head;
        while (prev->next != playlist->current) {
            prev = prev->next;
        }
        playlist->current = prev;
    }

    playCurrentSong(playlist);
}

//停止播放当前歌曲[简单的MCI命令封装下] [已查] 
void stopCurrentSong() {  
    mciSendString("close mymusic", NULL, 0, NULL);
}

//封装切换模式(1 - 4控制) [已查]
void changePlayMode(struct Playlist* playlist) {
    playlist->playMode = (enum PlayMode)((playlist->playMode + 1) % 4);
}

// === 界面绘制函数 ===

//列表绘制[含标题框,各种歌单展示,滚动条设计] [已查]
void drawSongList(struct Playlist* playlist, int showOnlyFavorites, int startX, int startY) {
    if (!playlist || !playlist->head) return;   //判空

    settextstyle(16, 0, "宋体");
    setbkmode(TRANSPARENT);                     //字体背景设计为空[去除字体白背景]

    // 绘制标题
    settextcolor(YELLOW);
    char title[100];

    //正常和收藏下的变化
    int favCount = countFavorites(playlist);
    if (showOnlyFavorites) sprintf_s(title, sizeof(title), "收藏歌曲 (共%d首)", favCount);
    else{
        sprintf_s(title, sizeof(title), "我的歌单 (共%d首，收藏:%d)",
            playlist->totalSongs, favCount);
}

    // 计算标题宽度
    int titleWidth = textwidth(title);

    // ===== 标题框参数 =====
    
    // 标题框左右边距
    int titleBoxLeft = startX - 15;     
    int titleBoxRight = startX + titleWidth + 105; 
    int titleBoxTop = startY + 10;         
    int titleBoxBottom = startY - 27;  

    // 绘制标题背景 
    setfillcolor(RGB(40, 40, 60));
    setlinecolor(RGB(100, 100, 150));
    setlinestyle(PS_SOLID, 2);

    //绘制圆角矩形
    solidroundrect(titleBoxLeft, titleBoxTop, titleBoxRight, titleBoxBottom, 8, 8);   //填充
    roundrect(titleBoxLeft, titleBoxTop, titleBoxRight, titleBoxBottom, 8, 8);        //空心

    // 绘制标题 - 居中显示
    int titleX = titleBoxLeft + (titleBoxRight - titleBoxLeft - titleWidth) / 2; 
    outtextxy(titleX, startY - 17, title);  

    // ===== 滚动提示位置 =====
    if (playlist->scrollOffset > 0 || playlist->totalSongs > playlist->visibleCount) {
        settextstyle(12, 0, "宋体");
        const char* scrollHint = "中键 滚动";
        int hintWidth = textwidth(scrollHint);

        // 滚动提示框位置
        int hintBoxLeft = startX + 305;    
        int hintBoxRight = hintBoxLeft + hintWidth + 8; 

        setfillcolor(RGB(80, 80, 100));   //设计填充色
        setlinecolor(RGB(120, 120, 150)); //设计线条颜色

        solidroundrect(hintBoxLeft, startY - 28, hintBoxRight, startY - 8, 5, 5);
        roundrect(hintBoxLeft, startY - 28, hintBoxRight, startY - 8, 5, 5);

        settextcolor(LIGHTGRAY);
        outtextxy(hintBoxLeft + 5, startY - 23, scrollHint);  //输出文本位置
        settextstyle(16, 0, "宋体");
    }

    // ===== 歌曲列表起始位置 =====
    int listStartY = startY + 10; 

    // ===== 滚动条位置和尺寸 =====
    int scrollBarLeft = startX + 305;   // 滚动条左侧位置
    int scrollBarRight = scrollBarLeft + 8;  // 滚动条宽度8像素
    int scrollBarHeight = playlist->visibleCount * 28;  // 滚动条高度

    // 绘制滚动条背景
    setfillcolor(RGB(70, 70, 90));  // 稍微亮一点的颜色
    solidrectangle(scrollBarLeft, listStartY,
        scrollBarRight, listStartY + scrollBarHeight);

    // 绘制滚动条滑块
    if (playlist->totalSongs > playlist->visibleCount) {
        // 计算滑块高度
        float visibleRatio = (float)playlist->visibleCount / playlist->totalSongs;
        int sliderHeight = (int)(scrollBarHeight * visibleRatio);
        if (sliderHeight < 20) sliderHeight = 20;  // 最小高度

        // 计算滑块位置
        float scrollRatio = (float)playlist->scrollOffset / (playlist->totalSongs - playlist->visibleCount);
        int sliderY = listStartY + (int)((scrollBarHeight - sliderHeight) * scrollRatio);


        setfillcolor(RGB(150, 150, 180));  
        solidrectangle(scrollBarLeft, sliderY, scrollBarRight, sliderY + sliderHeight);

        // 绘制滑块边框
        setlinecolor(RGB(180, 180, 210));
        rectangle(scrollBarLeft, sliderY, scrollBarRight, sliderY + sliderHeight);
    }

    // 绘制歌曲列表
    struct Song* current = playlist->head;
    int y = listStartY;  // 使用调整后的起始位置
    int displayIndex = 0;

    // 跳过滚动偏移
    for (int i = 0; i < playlist->scrollOffset && current; i++) {
        current = current->next;
    }

    // 重新定位current
    current = playlist->head;
    for (int i = 0; i < playlist->scrollOffset && current; i++) {
        current = current->next;
    }

    // 计算每行高度和间距
    int rowHeight = 28;  // 每行高度
    int rowSpacing = 2;  // 行间距

    while (current && displayIndex < playlist->visibleCount) {
        if (showOnlyFavorites && !current->favorite) {
            current = current->next;
            continue;
        }

        // 计算当前行矩形位置
        int rowTop = y;
        int rowBottom = y + rowHeight;
        int rowLeft = startX - 15;
        int rowRight = startX + 300;  // 列表宽度

        // 绘制行背景
        if (displayIndex % 2 == 0) {
            setfillcolor(RGB(50, 50, 60));  // 深色行
        }
        else {
            setfillcolor(RGB(55, 55, 65));  // 稍浅色行
        }
        solidrectangle(rowLeft, rowTop, rowRight, rowBottom);

        // 绘制行边框（细线）
        setlinecolor(RGB(70, 70, 85));
        setlinestyle(PS_SOLID, 1);
        rectangle(rowLeft, rowTop, rowRight, rowBottom);

        // 当前播放歌曲高亮
        if (current == playlist->current) {
            // 使用半透明叠加效果（通过绘制一个较亮的矩形）
            setfillcolor(RGB(40, 70, 110));
            setlinecolor(RGB(60, 100, 150));

            // 绘制高亮背景（稍微小一点，作为内边框效果）
            solidrectangle(rowLeft, rowTop + 2, rowRight - 2, rowBottom - 2);
            rectangle(rowLeft, rowTop + 2, rowRight - 2, rowBottom - 2);
        }

        // 准备显示文本
        char display[100];
        int textColor;

        if (showOnlyFavorites) {
            sprintf_s(display, sizeof(display), "%d.%s %s",
                displayIndex + 1,
                current->favorite ? "**" : "//",  // 收藏标记
                current->name);
            textColor = current->favorite ? LIGHTRED : LIGHTGRAY;
        }
        else {
            int index = playlist->scrollOffset + displayIndex + 1;
            sprintf_s(display, sizeof(display), "%d.%s %s",
                index,
                current->favorite ? "**" : "//",  // 收藏标记
                current->name);
            textColor = current->favorite ? LIGHTRED : LIGHTGRAY;
        }

        // 绘制歌曲名（限制宽度，防止超出）
        int maxTextWidth = 240;  // 最大文本宽度
        int actualTextWidth = textwidth(display);

        settextcolor(textColor);
        if (actualTextWidth > maxTextWidth) {
            // 如果文本太长，裁剪并添加省略号
            char clipped[100];
            strncpy_s(clipped, sizeof(clipped), display, 30);  // 截取前30字符
            strcat_s(clipped, sizeof(clipped), "...");
            outtextxy(rowLeft + 8, rowTop + 6, clipped);
        }
        else {
            outtextxy(rowLeft + 8, rowTop + 6, display);
        }


        // 移动到下一行
        current = current->next;
        y += rowHeight + rowSpacing;
        displayIndex++;

        // 跳过过滤的歌曲
        while (showOnlyFavorites && current && !current->favorite) {
            current = current->next;
        }
    }

    // 如果没有歌曲时的提示（调整位置）
    if (displayIndex == 0) {
        int centerX = startX + 150;  // 列表中心
        int centerY = listStartY + scrollBarHeight / 2;

        if (showOnlyFavorites) {
            settextcolor(LIGHTCYAN);
            outtextxy(centerX - 60, centerY - 20, "没有中意的歌曲呀..");
            settextcolor(RGB(180, 180, 200));
            outtextxy(centerX - 75, centerY + 5, "点击鼠标中键可以收藏");
        }
        else {
            settextcolor(LIGHTCYAN);
            outtextxy(centerX - 40, centerY - 20, "当前歌单为空");
            settextcolor(RGB(180, 180, 200));
            outtextxy(centerX - 70, centerY + 5, "请添加歌曲或重新扫描");
        }
    }

    // ===== 添加列表边框 =====
    setlinecolor(RGB(90, 90, 120));
    setlinestyle(PS_SOLID, 2);
    rectangle(startX - 15, listStartY - 2,
        startX + 302, listStartY + playlist->visibleCount * (rowHeight + rowSpacing) - rowSpacing + 2);
}

//绘制当前的歌曲信息[处理标题显示] [已查]
void drawCurrentSongInfo(struct Playlist* playlist, int x, int y) {
    if (!playlist || !playlist->current) return;

    // ===== 固定尺寸 =====
    const int BOX_WIDTH = 400;
    const int BOX_HEIGHT = 45;
    const int TEXT_MAX_CHARS = 25;  // 最大显示字符数

    // ===== 绘制固定大小的框 =====
    int boxX1 = x;
    int boxY1 = y;
    int boxX2 = boxX1 + BOX_WIDTH;
    int boxY2 = boxY1 + BOX_HEIGHT;

    setfillcolor(RGB(240, 240, 245));
    setlinecolor(RGB(180, 180, 200));
    setlinestyle(PS_SOLID, 2);

    solidroundrect(165, boxY1, boxX2, boxY2, 8, 8);
    roundrect(165, boxY1, boxX2, boxY2, 8, 8);

    // ===== 播放图标 =====
    setfillcolor(RGB(0, 180, 0));
    solidcircle(boxX1 + 25, boxY1 + BOX_HEIGHT / 2, 5);

    // ===== 准备显示文本 =====
    char displayText[256];
    strncpy_s(displayText, sizeof(displayText), playlist->current->name, 255);

    // 简单截断：超过指定字符数就加"..."
    if (strlen(displayText) > TEXT_MAX_CHARS) {
        displayText[TEXT_MAX_CHARS] = '\0';
        strcat_s(displayText, sizeof(displayText), "...");
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

//绘制当前的播放模式[已查]
void drawPlayModeInfo(struct Playlist* playlist, int x, int y) {
    settextstyle(16, 0, "宋体");

    const char* modeNames[] = { "顺序播放", "单曲循环", "列表循环", "随机播放" };
    const COLORREF modeColors[] = {
    RGB(0, 160, 70),     // 顺序播放 
    RGB(60, 100, 200),   // 单曲循环 
    RGB(0, 140, 140),    // 列表循环 
    RGB(160, 0, 160)     // 随机播放 
    };

    char modeText[50];
    sprintf_s(modeText, sizeof(modeText), "模式: %s", modeNames[playlist->playMode]);

    int textWidth = textwidth(modeText);
    int textHeight = textheight(modeText);
    setfillcolor(RGB(50, 50, 50));  // 深灰色背景
    setlinecolor(RGB(100, 100, 100));
    setlinestyle(PS_SOLID, 1);

    solidroundrect(x - 15, y - 5, x + textWidth + 5, y + textHeight + 5, 8, 8);
    roundrect(x - 15, y -5, x + textWidth + 5, y + textHeight + 5, 8, 8);

    setbkmode(TRANSPARENT);
    settextcolor(modeColors[playlist->playMode]);
    outtextxy(x, y, modeText);

    // 绘制提示信息背景
    settextstyle(14, 0, "宋体");
    const char* hintText = "[M键切换]";
    int hintWidth = textwidth(hintText);

    setfillcolor(RGB(80, 80, 80));  // 灰色背景
    setlinecolor(RGB(120, 120, 120));

    solidroundrect(x - 15, y + textHeight + 5, x + hintWidth + 5, y + textHeight + 25, 5, 5);
    roundrect(x - 15, y + textHeight + 5, x + hintWidth + 5, y + textHeight + 25, 5, 5);

    settextcolor(RGB(180, 180, 180));
    outtextxy(x, y + textHeight + 10, hintText);
}

// === 交互函数 ===

//鼠标控制歌单已添加[不同列表设计][按键设计][索引位置][已查]
int handleSongListClick(struct Playlist* playlist, int showOnlyFavorites, ExMessage msg, int startX, int startY) { 
    if (!playlist || !playlist->head) return 0;
    //判断歌单范围
    if (msg.x >= startX && msg.x <= startX + 300 &&
        msg.y >= startY && msg.y <= startY + playlist->visibleCount * 25) {

        int itemIndex = (msg.y - startY) / 28;

        struct Song* clickedSong = NULL;
        int originalIndex = -1;
     //判断当前播放形式
        if (showOnlyFavorites) {
            // 收藏模式下，计算点击的是第几首收藏歌曲
            struct Song* current = playlist->head;
            int favCount = 0;
            int displayCount = 0;

            // 跳过滚动偏移
            for (int i = 0; i < playlist->scrollOffset && current; i++) {
                if (current->favorite) favCount++;
                current = current->next;
            }

            // 重新开始遍历
            current = playlist->head;
            for (int i = 0; i < playlist->scrollOffset && current; i++) {
                current = current->next;
            }

            // 查找实际点击的收藏歌曲
            while (current && displayCount <= itemIndex) {
                if (current->favorite) {
                    if (displayCount == itemIndex) {
                        clickedSong = current;
                        // 找到原始索引
                        originalIndex = 0;
                        struct Song* temp = playlist->head;
                        while (temp != current) {
                            temp = temp->next;
                            originalIndex++;
                        }
                        break;
                    }
                    displayCount++;
                }
                current = current->next;
            }
        }
        else {
            // 正常模式下，按原来的方式计算
            int songIndex = playlist->scrollOffset + itemIndex;
            if (songIndex < playlist->totalSongs) {
                clickedSong = getSongAt(playlist, songIndex);
                originalIndex = songIndex;
            }
        }
        //判断点击,列表形式和歌曲索引后判断鼠标点击类型
        if (clickedSong) {
            if (msg.message == WM_LBUTTONDOWN) {
                playlist->current = clickedSong;
                playCurrentSong(playlist);
                return 1;   //播放
            }
            else if (msg.message == WM_RBUTTONDOWN) {
                if (originalIndex >= 0) {
                    removeSong(playlist, originalIndex);
                    return 2;  //删除
                }
            }
            else if (msg.message == WM_MBUTTONDOWN) {
                clickedSong->favorite = !clickedSong->favorite;
                savePlaylistToFile(playlist, PLAYLIST_FILE);
                return 3;    //收藏
            }
        }
    }

    return 0;
}

//正常模式下的滚动函数[已查]
void scrollPlaylist(struct Playlist* playlist, int direction) {
    if (!playlist) return;

    int newOffset = playlist->scrollOffset + direction;

    if (newOffset < 0) {
        newOffset = 0;
    }
    else if (newOffset > playlist->totalSongs - playlist->visibleCount) {
        newOffset = playlist->totalSongs - playlist->visibleCount;
    }

    if (newOffset < 0) newOffset = 0;

    playlist->scrollOffset = newOffset;
}

// 专门用于收藏模式的滚动函数[设计滚动使用][已查]
void scrollPlaylistFiltered(struct Playlist* playlist, int showOnlyFavorites, int direction) {
    if (!playlist) return;

    if (!showOnlyFavorites) {
        // 正常模式，使用原来的滚动函数
        scrollPlaylist(playlist, direction);
        return;
    }

    // 收藏模式下的滚动
    int favCount = countFavorites(playlist);
    if (favCount <= playlist->visibleCount) {
        playlist->scrollOffset = 0;  // 收藏歌曲少于一页，不需要滚动
        return;
    }

    // 计算新的滚动偏移
    // 找到当前滚动偏移后第一个收藏歌曲的实际索引
    struct Song* current = playlist->head;
    int actualIndex = 0;
    int favFound = 0;

    for (int i = 0; i < playlist->scrollOffset && current; i++) {
        if (current->favorite) favFound++;
        current = current->next;
    }

    // 计算新的偏移
    int newFavOffset = favFound + direction;
    if (newFavOffset < 0) newFavOffset = 0;
    if (newFavOffset > favCount - playlist->visibleCount) {
        newFavOffset = favCount - playlist->visibleCount;
    }

    // 根据新的收藏偏移找到实际偏移
    if (newFavOffset == 0) {
        playlist->scrollOffset = 0;
        return;
    }

    // 找到第newFavOffset个收藏歌曲的实际位置
    current = playlist->head;
    int currentIndex = 0;
    favFound = 0;

    while (current && favFound < newFavOffset) {
        if (current->favorite) favFound++;
        current = current->next;
        if (favFound < newFavOffset) currentIndex++;
    }

    playlist->scrollOffset = currentIndex;
}

//扫描本地文件,添加保存歌单[已查]
void scanMusicFolder(struct Playlist* playlist, const char* folderPath) {
    // 检查音乐文件夹是否存在，不存在则创建
    if (_access(folderPath, 0) == -1) {
        int mkdirResult = _mkdir(folderPath);
        if (mkdirResult == -1) {
            // 获取错误信息
            char errorMsg[256];
            strerror_s(errorMsg, sizeof(errorMsg), errno);   //字符串错误函数
            printf("额,本地创建文件夹失败: %s (错误: %s)\n", folderPath, errorMsg);
            return;
        }   //创建文件夹可能失败 ( 注意 ); 
        printf("文件不存在,已自动创建(￣-￣): %s\n", folderPath);
        return;
    }

    struct _finddata_t fileinfo;
    intptr_t handle;
    char searchPath[512];

    // 构建搜索路径 ( 搭配_findfirst函数使用 ,*泛搜 )
    sprintf_s(searchPath, sizeof(searchPath), "%s/*.mp3", folderPath);

    // 开始查找第一个文件
    handle = _findfirst(searchPath, &fileinfo);
    if (handle == -1) {
        printf("可恶::>_<::, 在本地 %s 中没有找到MP3文件\n", folderPath);
        return;
    }

    printf("来咯, 我要扫描音乐文件夹: %s 喽\n", folderPath);

    int scannedCount = 0;

    do {
        // 跳过子目录，只处理文件
        if (fileinfo.attrib & _A_SUBDIR) {
            continue;
        }

        // 获取文件名
        char* filename = fileinfo.name;

        // 获取歌曲名字
        char songName[256];
        strcpy_s(songName, sizeof(songName), filename);

        // 去除文件后缀 , 找到最后一个点号的位置
        char* dot = strrchr(songName, '.');  //strrchr返回搜寻字符最后一个指针 ; 区分 strchr返回第一个  ;
        if (dot) {
            *dot = '\0';  // 去掉扩展名
        }

        // 构建完整路径 ( 实现具体实操文件,对个体文件 )
        char fullPath[512];
        sprintf_s(fullPath, sizeof(fullPath), "%s/%s", folderPath, filename);

        // 检查是否已经存在于歌单中
        int alreadyExists = 0;
        struct Song* current = playlist->head;
        while (current) {
            if (strcmp(current->name, songName) == 0 ||
                strcmp(current->path, fullPath) == 0) {
                alreadyExists = 1;
                break;
            }
            current = current->next;
        }

        // 如果不存在，添加到歌单
        if (!alreadyExists) {
            addSong(playlist, songName, fullPath);
            scannedCount++;
            printf("接着奏的乐曲是: %s\n", songName);
        }

    } while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);   //释放系统资源

    if (scannedCount > 0) {
        printf("成功奏上 %d 首新歌曲\n", scannedCount);
        // 保存更新后的歌单到文件
        savePlaylistToFile(playlist, PLAYLIST_FILE);
    }
    else {
        printf("哎~,没有发现新歌曲\n");
    }
}