#define _CRT_SECURE_NO_WARNINGS
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#include "player.h"

#pragma comment(lib, "winmm.lib")


int main() {

    srand((unsigned int)time(NULL));

    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

   /* if (!playIntroVideo()) {
        return 0;
    }
    cleardevice();*/

    IMAGE img;
    loadimage(&img, "./source/picture/background.jpg", WINDOW_WIDTH, WINDOW_HEIGHT);

    struct Playlist playlist;

    initPlaylist(&playlist);
    g_playlist = &playlist;

    playlist.volume = 80;

    loadDefaultPlaylist(&playlist);   

    if (playlist.musicSource == MUSIC_SOURCE_AUTO) {
        playlist.visibleSongs = playlist.totalSongs;
    }
   
    
    // 创建左侧按钮
    struct Button* lyricSourceBtn = creatButton(20, 515, 120, 40, RGB(150, 150, 255), "歌词:自动", 11);
    struct Button* musicSourceBtn = creatButton(20, 560, 120, 40, RGB(150, 200, 255), "音乐:自动", 12);
    struct Button* play = creatButton(20, 20, 120, 40, RGB(100, 200, 100), "播放", 0);
    struct Button* pause = creatButton(20, 65, 120, 40, RGB(200, 200, 100), "暂停", 1);
    struct Button* stop = creatButton(20, 110, 120, 40, RGB(200, 100, 100), "停止", 2);
    struct Button* prev = creatButton(20, 155, 120, 40, RGB(100, 200, 200), "上一首", 3);
    struct Button* next = creatButton(20, 200, 120, 40, RGB(100, 200, 200), "下一首", 4);
    struct Button* mode = creatButton(20, 245, 120, 40, RGB(200, 100, 200), "播放模式", 5);
    struct Button* addSongBtn = creatButton(20, 290, 120, 40, RGB(150, 150, 255), "添加歌曲", 6);
    struct Button* showFavBtn = creatButton(20, 335, 120, 40, RGB(255, 200, 100), "显示收藏", 7);
    struct Button* clearAllBtn = creatButton(20, 380, 120, 40, RGB(255, 100, 100), "清空歌单", 8);
    struct Button* saveBtn = creatButton(20, 425, 120, 40, RGB(100, 255, 100), "保存歌单", 9);
    struct Button* rescanBtn = creatButton(20, 470, 120, 40, RGB(150, 200, 255), "重新扫描", 10);
    const char* musicSourceTexts[] = { "音乐:默认", "音乐:用户", "音乐:自动" };
    SetTextButton(musicSourceBtn, musicSourceTexts[playlist.musicSource]);
    // 创建右侧圆形控制按钮
    int controlPanelX = WINDOW_WIDTH - 400;
    int controlPanelY = WINDOW_HEIGHT - 120;

    struct Button* prevCircle = createCircleButton(controlPanelX - 100, controlPanelY + 60, 30,
        RGB(100, 200, 200), "<-", 20);
    struct Button* playPauseCircle = createCircleButton(controlPanelX, controlPanelY + 60, 38,
        RGB(100, 200, 100), "O", 21);
    struct Button* nextCircle = createCircleButton(controlPanelX + 100, controlPanelY + 60, 30,
        RGB(100, 200, 200), "->", 22);


    BeginBatchDraw();

    ExMessage msg;
    int pauseFlag = 1;
    int showOnlyFavorites = 0;

    // 计算右侧歌词区域位置
    int lyricX = WINDOW_WIDTH - 550;
    int lyricY = 120;
    int lyricWidth = 500;
    int lyricHeight = 380;

    // 计算进度条位置
    int progressX = lyricX;
    int progressY = lyricY + lyricHeight + 25;
    int progressWidth = lyricWidth;
    int progressHeight = 18;

    // 计算音量控制位置
    int volumeX = progressX;
    int volumeY = progressY + 50;
    int volumeWidth = 250;
    int volumeHeight = 18;

    // 状态变量
    int currentMusicY = 200;

    while (1) {
        //[新增]放弃MCI,入侵太厉害了,转向可视化进度条[>_<]
        static int play_complete_count = 0;    //设计帧数
        if (g_playlist && g_playlist->current) {
            char posStr[32] = { 0 };
            char lenStr[32] = { 0 };

            mciSendString("status mymusic position", posStr, sizeof(posStr), NULL);   //获取播放歌曲位置
            mciSendString("status mymusic length", lenStr, sizeof(lenStr), NULL);    //获取歌曲总长度
            //[核心]计算百分比
            int current_ms = atoi(posStr);   //将MCI传回的数据换算为毫秒
            int total_ms = atoi(lenStr);     //完整的毫秒数

            if (total_ms > 0) {
                int progress = (current_ms * 100) / total_ms;

                if (progress >= 99) {   //播放进度≥99%时启动切歌倒计时,倒计时采用帧计数方式，每帧计数+1,不同设备帧数不同可以自行调整停留帧数
                    play_complete_count++;  //帧数++ 
                    if (play_complete_count >= 100) {      //此处可以按需调整自己帧数
                        playNextSong(g_playlist);
                        play_complete_count = 0;
                    }
                }
                else {
                    play_complete_count = 0;
                }
            }
        }
        putimage(0, 0, &img);

        // 绘制左侧歌单区域
        drawCurrentSongInfo(&playlist, 180, 50);
        drawPlayModeInfo(&playlist, 180, 100);
        drawSongList(&playlist, showOnlyFavorites, 180, 200);

        // 绘制右侧歌词区域
        drawLyric(&playlist, lyricX, lyricY, lyricWidth, lyricHeight);

        // 绘制进度条
        char lengthStr[64] = "0";
        mciSendString("status mymusic length", lengthStr, sizeof(lengthStr), NULL);
        int totalLength_ms = atoi(lengthStr);
        int currentPosition = getCurrentPlayPosition() * 1000;

        if (totalLength_ms > 0) {
            int progressPercent = (int)((float)currentPosition / totalLength_ms * 100);
            drawProgressBar(progressX, progressY, progressWidth, progressHeight, progressPercent);
        }

        // 绘制音量控制
        drawVolumeBar(&playlist, volumeX, volumeY, volumeWidth, volumeHeight);

        // 更新歌词位置
        updateLyricPosition(&playlist);

        // 显示所有按钮
        Show(play);
        Show(pause);
        Show(stop);
        Show(prev);
        Show(next);
        Show(mode);
        Show(addSongBtn);
        Show(showFavBtn);
        Show(clearAllBtn);
        Show(saveBtn);
        Show(rescanBtn);
        Show(lyricSourceBtn);
        Show(musicSourceBtn);
        Show(prevCircle);
        Show(playPauseCircle);
        Show(nextCircle);

        // 绘制状态信息
        settextstyle(16, 0, "宋体");
        settextcolor(LIGHTGRAY);
        setbkmode(TRANSPARENT);

        // 绘制歌词源信息
        char lyricSourceMsg[50];
        const char* lyricSourceNames[] = { "默认", "用户", "自动" };
        sprintf(lyricSourceMsg, "歌词源: %s", lyricSourceNames[playlist.lyricSource]);

        int lyricSourceWidth = textwidth(lyricSourceMsg);
        setfillcolor(RGB(50, 50, 80));
        setlinecolor(RGB(100, 100, 150));
        setlinestyle(PS_SOLID, 2);
        int lyricSourceY = 125;
        solidroundrect(373, 95, 565, 122, 8, 8);
        roundrect(373, 95, 565, 122, 8, 8);

        settextcolor(LIGHTBLUE);
        outtextxy(383, 99, lyricSourceMsg);
        // 绘制音乐源信息
        settextstyle(14, 0, "宋体");
        settextcolor(LIGHTGRAY);
        setbkmode(TRANSPARENT);
        char musicSourceMsg[50];
        const char* musicSourceNames[] = { "默认", "用户", "自动" };
        sprintf(musicSourceMsg, "音乐源: %s", musicSourceNames[playlist.musicSource]);

        int musicSourceWidth = textwidth(musicSourceMsg);
        int musicSourceY = lyricSourceY + 30;
        solidroundrect(373, 122, 565, 145, 8, 8);
        roundrect(373, 122, 565, 145, 8, 8);

        settextcolor(LIGHTCYAN);
        outtextxy(383, 127, musicSourceMsg);

        // 绘制当前歌曲信息
        if (playlist.current) {
            char currentMusicMsg[100];
            enum MusicSource currentMusicSource = getMusicSourceForSong(&playlist, playlist.current);
            const char* musicSourceText = "未知";

            if (playlist.current->path) {
                if (strstr(playlist.current->path, USER_MUSIC_FOLDER)) {
                    musicSourceText = "用户文件夹";
                }
                else if (strstr(playlist.current->path, DEFAULT_MUSIC_FOLDER)) {
                    musicSourceText = "默认文件夹";
                }
            }

            sprintf(currentMusicMsg, "当前音乐: %s", musicSourceText);

            int currentMusicWidth = textwidth(currentMusicMsg);
            setfillcolor(RGB(40, 40, 60));
            setlinecolor(RGB(80, 80, 120));

            currentMusicY = musicSourceY + 30;
            solidroundrect(373, 145, 565, 172, 8, 8);
            roundrect(373, 145, 565, 172, 8, 8);

            settextcolor(LIGHTBLUE);
            outtextxy(383, 150, currentMusicMsg);
        }

        // 绘制状态信息
        char statusMsg[100];
        if (showOnlyFavorites) {
            int favCount = countFavorites(&playlist);
            sprintf(statusMsg, "当前: 显示收藏歌曲 (%d首)", favCount);
        }
        else {
            sprintf(statusMsg, "当前: 显示全部歌曲 (%d首)", playlist.totalSongs);
        }
        setfillcolor(RGB(50, 50, 80));
        setlinecolor(RGB(100, 100, 150));
        setlinestyle(PS_SOLID, 2);
        solidroundrect(165, 143, 373, 173, 8, 8);
        roundrect(165, 143, 373, 173, 8, 8);

        settextcolor(LIGHTGREEN);
        outtextxy(180, 150, statusMsg);

        // 绘制操作提示
        settextstyle(16, 0, "宋体");
        settextcolor(LIGHTGRAY);
        setbkmode(TRANSPARENT);
        setfillcolor(RGB(40, 40, 60));
        setlinecolor(RGB(80, 80, 120));
        setlinestyle(PS_SOLID, 2);

        solidroundrect(15, 605, 140, 715, 10, 10);
        roundrect(15, 605, 140, 715, 10, 10);

        setfillcolor(RGB(60, 60, 90));
        solidroundrect(20, 610, 140, 632, 5, 5);
        roundrect(20, 610, 140, 632, 5, 5);

        settextcolor(LIGHTGREEN);
        outtextxy(20, 612, "操作提示:");

        settextcolor(LIGHTGRAY);
        outtextxy(20, 635, "左键: 播放歌曲");
        outtextxy(20, 655, "右键: 删除歌曲");
        outtextxy(20, 675, "中键: 切换收藏");
        outtextxy(20, 695, "滚轮: 滚动歌单");

        FlushBatchDraw();

        if (peekmessage(&msg)) {
            // 鼠标悬停检测
            InButton(play, msg);
            InButton(pause, msg);
            InButton(stop, msg);
            InButton(prev, msg);
            InButton(next, msg);
            InButton(mode, msg);
            InButton(addSongBtn, msg);
            InButton(showFavBtn, msg);
            InButton(clearAllBtn, msg);
            InButton(saveBtn, msg);
            InButton(rescanBtn, msg);
            InButton(lyricSourceBtn, msg);
            InButton(musicSourceBtn, msg);
            InButton(prevCircle, msg);
            InButton(playPauseCircle, msg);
            InButton(nextCircle, msg);

            // 处理歌词区域滚轮
            if (msg.message == WM_MOUSEWHEEL &&
                msg.x >= lyricX && msg.x <= lyricX + lyricWidth &&
                msg.y >= lyricY && msg.y <= lyricY + lyricHeight) {

                if (playlist.current && playlist.current->lyric) {
                    if (msg.wheel < 0) {
                        scrollLyric(playlist.current->lyric, 1);
                    }
                    else {
                        scrollLyric(playlist.current->lyric, -1);
                    }
                }
            }

            // 处理进度条点击
            handleProgressBarClick(msg, progressX, progressY, progressWidth, progressHeight);

            // 处理音量控制点击
            handleVolumeBarClick(&playlist, msg, volumeX, volumeY, volumeWidth, volumeHeight);

            // 处理歌单点击
            int songClickResult = handleSongListClick(&playlist, showOnlyFavorites, msg, 180, 200);
            if (songClickResult > 0) {
                if (songClickResult == 1) {
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                }
                continue;
            }

            // 处理右侧圆形按钮点击
            if (msg.message == WM_LBUTTONDOWN) {
                if (InButton(prevCircle, msg)) {
                    playPrevSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                }
                else if (InButton(playPauseCircle, msg)) {
                    if (pauseFlag == 0) {
                        mciSendString("pause mymusic", 0, 0, 0);
                        pauseFlag = 1;
                        SetTextButton(playPauseCircle, "-");
                        SetTextButton(pause, "继续");
                    }
                    else {
                        mciSendString("resume mymusic", 0, 0, 0);
                        pauseFlag = 0;
                        SetTextButton(playPauseCircle, "||");
                        SetTextButton(pause, "暂停");
                    }
                }
                else if (InButton(nextCircle, msg)) {
                    playNextSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                }
            }

            // 歌词源按钮处理
            if (msg.message == WM_LBUTTONDOWN && InButton(lyricSourceBtn, msg)) {
                enum LyricSource newSource = (enum LyricSource)((playlist.lyricSource + 1) % 3);
                setLyricSource(&playlist, newSource);
                const char* sourceTexts[] = { "歌词:默认", "歌词:用户", "歌词:自动" };
                SetTextButton(lyricSourceBtn, sourceTexts[newSource]);
                if (playlist.current) {
                    reloadLyricsForCurrentSong(&playlist);
                }
            }

            // 音乐源按钮处理
            if (msg.message == WM_LBUTTONDOWN && InButton(musicSourceBtn, msg)) {
                enum MusicSource newSource = (enum MusicSource)((playlist.musicSource + 1) % 3);
                setMusicSource(&playlist, newSource);

                const char* sourceTexts[] = { "音乐:默认", "音乐:用户", "音乐:自动" };
                SetTextButton(musicSourceBtn, sourceTexts[newSource]);

                cleardevice();
                putimage(0, 0, &img);

                if (playlist.current) {
                    playCurrentSong(&playlist);
                    if (pauseFlag == 0) {
                        mciSendString("pause mymusic", 0, 0, 0);
                    }
                }

                FlushBatchDraw();
                continue;  // 跳过本次循环的其他处理
            }

            // "显示收藏"按钮处理
            if (msg.message == WM_LBUTTONDOWN && InButton(showFavBtn, msg)) {
                showOnlyFavorites = !showOnlyFavorites;
                playlist.scrollOffset = 0;

                if (showOnlyFavorites) {
                    SetTextButton(showFavBtn, "显示全部");
                    if (playlist.current && !playlist.current->favorite) {
                        struct Song* firstFav = playlist.head;
                        while (firstFav && !firstFav->favorite) {
                            firstFav = firstFav->next;
                        }
                        if (firstFav) {
                            playlist.current = firstFav;
                        }
                    }
                }
                else {
                    SetTextButton(showFavBtn, "显示收藏");
                }
            }

            // 其他按钮点击处理
            if (msg.message == WM_LBUTTONDOWN) {
                if (InButton(play, msg)) {
                    mciSendString("play mymusic", 0, 0, 0);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                }
                else if (InButton(pause, msg)) {
                    if (pauseFlag == 0) {
                        mciSendString("pause mymusic", 0, 0, 0);
                        pauseFlag = 1;
                        SetTextButton(pause, "继续");
                        SetTextButton(playPauseCircle, "-");
                    }
                    else {
                        mciSendString("resume mymusic", 0, 0, 0);
                        pauseFlag = 0;
                        SetTextButton(pause, "暂停");
                        SetTextButton(playPauseCircle, "||");
                    }
                }
                else if (InButton(stop, msg)) {
                    mciSendString("stop mymusic", 0, 0, 0);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "-");
                }
                else if (InButton(prev, msg)) {
                    playPrevSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                }
                else if (InButton(next, msg)) {
                    playNextSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                }
                else if (InButton(mode, msg)) {
                    changePlayMode(&playlist);
                }
                else if (InButton(addSongBtn, msg)) {
                    MessageBox(NULL,
                        "请将歌曲文件放入以下目录：\n"
                        "默认音乐: ./source/music/default/\n"
                        "用户音乐: ./source/music/user/\n\n"
                        "歌词文件放入以下目录：\n"
                        "默认歌词: ./source/lyrics/default/\n"
                        "用户歌词: ./source/lyrics/user/",
                        "文件存放提示", MB_OK | MB_ICONINFORMATION);
                }
                else if (InButton(clearAllBtn, msg)) {
                    int result = MessageBox(NULL, "确定要清空整个歌单吗？", "确认", MB_OKCANCEL | MB_ICONQUESTION);
                    if (result == IDOK) {
                        removeAllSongs(&playlist);
                    }
                }
                else if (InButton(saveBtn, msg)) {
                    saveUserPlaylist(&playlist);
                    settextstyle(16, 0, "宋体");
                    settextcolor(LIGHTGREEN);
                    outtextxy(400, 660, "歌单已保存到 user_playlist.txt");
                    FlushBatchDraw();
                    Sleep(1000);
                }
                else if (InButton(rescanBtn, msg)) {
                    char status[64] = "";
                    mciSendString("status mymusic mode", status, sizeof(status), 0);
                    int wasPlaying = (strcmp(status, "playing") == 0);
                    struct Song* currentBeforeScan = playlist.current;

                    settextstyle(16, 0, "宋体");
                    settextcolor(LIGHTGREEN);

                    const char* scanMsg = "正在扫描音乐文件夹...";
                    int msgWidth = textwidth(scanMsg);

                    setfillcolor(RGB(30, 60, 30));
                    setlinecolor(RGB(50, 100, 50));
                    setlinestyle(PS_SOLID, 2);

                    solidroundrect(395, 655, 395 + msgWidth + 10, 685, 5, 5);
                    roundrect(395, 655, 395 + msgWidth + 10, 685, 5, 5);

                    outtextxy(400, 660, scanMsg);
                    FlushBatchDraw();

                    scanMusicFolders(&playlist);

                    if (wasPlaying && playlist.current) {
                        playCurrentSong(&playlist);
                    }
                    else if (currentBeforeScan) {
                        playlist.current = currentBeforeScan;
                    }

                    FlushBatchDraw();
                    Sleep(1000);
                }
            }

            // 键盘快捷键处理
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case VK_SPACE:
                    if (pauseFlag == 0) {
                        mciSendString("pause mymusic", 0, 0, 0);
                        pauseFlag = 1;
                        SetTextButton(pause, "继续");
                        SetTextButton(playPauseCircle, "-");
                    }
                    else {
                        mciSendString("resume mymusic", 0, 0, 0);
                        pauseFlag = 0;
                        SetTextButton(pause, "暂停");
                        SetTextButton(playPauseCircle, "||");
                    }
                    break;

                case VK_LEFT:
                    playPrevSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                    break;

                case VK_RIGHT:
                    playNextSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    SetTextButton(playPauseCircle, "||");
                    break;

                case 'M':
                case 'm':
                    changePlayMode(&playlist);
                    break;

                case 'K':
                case 'k':
                {
                    enum LyricSource newSource = (enum LyricSource)((playlist.lyricSource + 1) % 3);
                    setLyricSource(&playlist, newSource);
                    const char* sourceTexts[] = { "歌词:默认", "歌词:用户", "歌词:自动" };
                    SetTextButton(lyricSourceBtn, sourceTexts[newSource]);
                    if (playlist.current) {
                        reloadLyricsForCurrentSong(&playlist);
                    }
                }
                break;

                case 'J':
                case 'j':
                {
                    enum MusicSource newSource = (enum MusicSource)((playlist.musicSource + 1) % 3);
                    setMusicSource(&playlist, newSource);
                    const char* sourceTexts[] = { "音乐:默认", "音乐:用户", "音乐:自动" };
                    SetTextButton(musicSourceBtn, sourceTexts[newSource]);
                    if (playlist.current) {
                        playCurrentSong(&playlist);
                        if (pauseFlag == 0) {
                            mciSendString("pause mymusic", 0, 0, 0);
                        }
                    }
                }
                break;

                case VK_UP:
                    if (playlist.volume < 100) {
                        playlist.volume += 5;
                        if (playlist.volume > 100) playlist.volume = 100;
                        char cmd[64];
                        sprintf(cmd, "setaudio mymusic volume to %d", playlist.volume * 10);
                        mciSendString(cmd, NULL, 0, NULL);
                    }
                    break;

                case VK_DOWN:
                    if (playlist.volume > 0) {
                        playlist.volume -= 5;
                        if (playlist.volume < 0) playlist.volume = 0;
                        char cmd[64];
                        sprintf(cmd, "setaudio mymusic volume to %d", playlist.volume * 10);
                        mciSendString(cmd, NULL, 0, NULL);
                    }
                    break;

                case VK_ESCAPE:
                    goto exit_loop;
                }
            }

            // 鼠标滚轮处理
            if (msg.message == WM_MOUSEWHEEL) {
                if (msg.wheel < 0) {
                    if (showOnlyFavorites) {
                        scrollPlaylistFiltered(&playlist, showOnlyFavorites, 1);
                    }
                    else {
                        scrollPlaylist(&playlist, 1);
                    }
                }
                else {
                    if (showOnlyFavorites) {
                        scrollPlaylistFiltered(&playlist, showOnlyFavorites, -1);
                    }
                    else {
                        scrollPlaylist(&playlist, -1);
                    }
                }
            }
        }

        Sleep(10);
    }

exit_loop:
    EndBatchDraw();
    stopCurrentSong();
    saveUserPlaylist(&playlist);

    // 释放所有资源
    freePlaylist(&playlist);
    DestroyButton(play);
    DestroyButton(pause);
    DestroyButton(stop);
    DestroyButton(prev);
    DestroyButton(next);
    DestroyButton(mode);
    DestroyButton(addSongBtn);
    DestroyButton(showFavBtn);
    DestroyButton(clearAllBtn);
    DestroyButton(saveBtn);
    DestroyButton(rescanBtn);
    DestroyButton(lyricSourceBtn);
    DestroyButton(musicSourceBtn);
    DestroyButton(prevCircle);
    DestroyButton(playPauseCircle);
    DestroyButton(nextCircle);

    closegraph();
    printf("// 我想你可能想说拜拜！Ciallo~");

    return 0;
}