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
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#include "player.h"

#pragma comment(lib, "winmm.lib")

int main() {
    srand((unsigned int)time(NULL));  //(rand)利用时间生成随机种子 ->随机播放

    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    IMAGE img;
    loadimage(&img, "./source/picture/background.jpg", WINDOW_WIDTH, WINDOW_HEIGHT);
    struct Playlist playlist;
    initPlaylist(&playlist);

    // 创建按钮时添加id参数
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

    if (playlist.current) {
        playCurrentSong(&playlist);
        mciSendString("pause mymusic", 0, 0, 0);
    }

    BeginBatchDraw();           //实现双缓冲
    ExMessage msg;
    int pauseFlag = 1;           //控制暂停的状态值
    int showOnlyFavorites = 0;   //控制显示收藏的状态值

    while (1) {
        putimage(0, 0, &img);  //( x ,y , 地址  )

        drawCurrentSongInfo(&playlist, 180, 50);  //绘制歌单
        drawPlayModeInfo(&playlist, 180, 100);
        drawSongList(&playlist, showOnlyFavorites, 180, 200);

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

        settextstyle(16, 0, "宋体");
        settextcolor(LIGHTGRAY);
        setbkmode(TRANSPARENT);

        // 显示状态信息
        char statusMsg[100];
        if (showOnlyFavorites) {
            int favCount = countFavorites(&playlist);
            sprintf_s(statusMsg, sizeof(statusMsg), "状态: 显示收藏歌曲 (%d首)", favCount);
        }
        else {
            sprintf_s(statusMsg, sizeof(statusMsg), "状态: 显示全部歌曲 (%d首)", playlist.totalSongs);
        }

        // 计算状态信息宽度
        int statusWidth = textwidth(statusMsg);

        // 绘制状态信息背景
        setfillcolor(RGB(50, 50, 80));  // 紫色背景
        setlinecolor(RGB(100, 100, 150));
        setlinestyle(PS_SOLID, 2);

        solidroundrect(165, 145, 175 + statusWidth + 10, 170, 8, 8);
        roundrect(165, 145, 175 + statusWidth + 10, 170, 8, 8);

        // 绘制状态信息
        settextcolor(LIGHTGREEN);
        outtextxy(180, 150, statusMsg);

        //----添加内部操作实现提示----
        settextstyle(16, 0, "宋体");
        settextcolor(LIGHTGRAY);
        setbkmode(TRANSPARENT);
        setfillcolor(RGB(40, 40, 60)); 
        setlinecolor(RGB(80, 80, 120));
        setlinestyle(PS_SOLID, 2);

        // 绘制背景框
        solidroundrect(15, 515, 140, 715, 10, 10);
        roundrect(15, 515, 140, 715, 10, 10);

        // 绘制内部标题背景
        setfillcolor(RGB(60, 60, 90));
        solidroundrect(20, 520, 140, 542, 5, 5);
        roundrect(20, 520, 140, 542, 5, 5);

        // 绘制操作提示标题
        settextcolor(LIGHTGREEN);
        outtextxy(20, 522, "操作提示:");

        // 绘制操作提示内容
        settextcolor(LIGHTGRAY);
        outtextxy(20, 545, "左键: 播放歌曲");
        outtextxy(20, 565, "右键: 删除歌曲");
        outtextxy(20, 585, "中键: 切换收藏");
        outtextxy(20, 605, "滚轮: 滚动歌单");

        // 绘制快捷键标题背景
        setfillcolor(RGB(60, 60, 90));
        solidroundrect(20, 625, 140, 645, 5, 5);
        roundrect(20, 625, 140, 645, 5, 5);

        settextcolor(LIGHTGREEN);
        outtextxy(20, 627, "快捷键:");

        settextcolor(LIGHTGRAY);
        outtextxy(20, 650, "空格=播放/暂停");
        outtextxy(20, 670, "M=切换模式");
        outtextxy(20, 690, "ESC=退出程序");

        FlushBatchDraw();

        if (peekmessage(&msg)) {
            // 判断鼠标悬停位置
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

            // 处理歌单点击（传递 showOnlyFavorites 参数）[ 1-播放, 2-删除 , 3-收藏 ]
            int songClickResult = handleSongListClick(&playlist, showOnlyFavorites, msg, 240, 200);

            if (songClickResult > 0) {
                if (songClickResult == 1) {
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                }
                continue;
            }

            // "显示收藏"按钮处理
            if (msg.message == WM_LBUTTONDOWN && InButton(showFavBtn, msg)) {
                showOnlyFavorites = !showOnlyFavorites;

                // 重置滚动偏移
                playlist.scrollOffset = 0;

                if (showOnlyFavorites) {
                    SetTextButton(showFavBtn, "显示全部");

                    // 如果当前歌曲不是收藏，找到第一首收藏歌曲
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

            // 其他按钮点击处理[后期处理封装播放状态管理]
            if (msg.message == WM_LBUTTONDOWN) {
                if (InButton(play, msg)) {
                    mciSendString("play mymusic", 0, 0, 0);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                }
                else if (InButton(pause, msg)) {
                    if (pauseFlag == 0) {
                        mciSendString("pause mymusic", 0, 0, 0);
                        pauseFlag = 1;
                        SetTextButton(pause, "继续");
                    }
                    else {
                        mciSendString("resume mymusic", 0, 0, 0);
                        pauseFlag = 0;
                        SetTextButton(pause, "暂停");
                    }
                }
                else if (InButton(stop, msg)) {
                    mciSendString("stop mymusic", 0, 0, 0);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                }
                else if (InButton(prev, msg)) {
                    playPrevSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                }
                else if (InButton(next, msg)) {
                    playNextSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                }
                else if (InButton(mode, msg)) {
                    changePlayMode(&playlist);
                }
                else if (InButton(addSongBtn, msg)) {
                    MessageBox(NULL, "请将歌曲文件放入source/music目录", "提示", MB_OK | MB_ICONINFORMATION);
                }
                else if (InButton(clearAllBtn, msg)) {
                    int result = MessageBox(NULL, "确定要清空整个歌单吗？", "确认", MB_OKCANCEL | MB_ICONQUESTION);
                    if (result == IDOK) {
                        removeAllSongs(&playlist);
                    }
                }
                else if (InButton(saveBtn, msg)) {
                    savePlaylistToFile(&playlist, "playlist.txt");

                    settextstyle(16, 0, "宋体");
                    settextcolor(LIGHTGREEN);
                    outtextxy(400, 660, "歌单已保存到 playlist.txt");
                    FlushBatchDraw();
                    Sleep(1000);
                }
                else if (InButton(rescanBtn, msg)) {
                    // 先备份当前播放状态
                    char status[64];
                    mciSendString("status mymusic mode", status, sizeof(status), 0);
                    int wasPlaying = (strcmp(status, "playing") == 0);
                    struct Song* currentBeforeScan = playlist.current;

                    // 显示扫描消息时
                    settextstyle(16, 0, "宋体");
                    settextcolor(LIGHTGREEN);

                    // 计算消息宽度
                    const char* scanMsg = "正在扫描音乐文件夹...";
                    int msgWidth = textwidth(scanMsg);

                    // 绘制消息背景
                    setfillcolor(RGB(30, 60, 30));  // 绿色背景
                    setlinecolor(RGB(50, 100, 50));
                    setlinestyle(PS_SOLID, 2);

                    solidroundrect(395, 655, 395 + msgWidth + 10, 685, 5, 5);
                    roundrect(395, 655, 395 + msgWidth + 10, 685, 5, 5);

                    outtextxy(400, 660, scanMsg);
                    FlushBatchDraw();

                    // 扫描本地目标文件夹 (已查)
                    scanMusicFolder(&playlist, "./source/music");

                    // 恢复当前播放状态
                    if (wasPlaying && playlist.current) {
                        playCurrentSong(&playlist);
                    }
                    else if (currentBeforeScan) {
                        // 保持之前播放的歌曲
                        playlist.current = currentBeforeScan;
                    }

                    FlushBatchDraw();
                    Sleep(1000);
                }
            }

            // 键盘快捷键处理[内部悄悄实现][已查]
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case VK_SPACE:  //[空格]
                    if (pauseFlag == 0) {
                        mciSendString("pause mymusic", 0, 0, 0);
                        pauseFlag = 1;
                        SetTextButton(pause, "继续");
                    }
                    else {
                        mciSendString("resume mymusic", 0, 0, 0);
                        pauseFlag = 0;
                        SetTextButton(pause, "暂停");
                    }
                    break;

                case VK_LEFT:   //方向键
                    playPrevSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    break;

                case VK_RIGHT:    //方向键
                    playNextSong(&playlist);
                    pauseFlag = 0;
                    SetTextButton(pause, "暂停");
                    break;      
                case 'm':       //m键
                    changePlayMode(&playlist);
                    break;

                case VK_UP:           //方向键设计滚动
                    if (showOnlyFavorites) {
                        // 收藏模式下使用自定义滚动逻辑
                        scrollPlaylistFiltered(&playlist, showOnlyFavorites, -1);
                    }
                    else {
                        scrollPlaylist(&playlist, -1);
                    }
                    break;

                case VK_DOWN:
                    if (showOnlyFavorites) {
                        scrollPlaylistFiltered(&playlist, showOnlyFavorites, 1);
                    }
                    else {
                        scrollPlaylist(&playlist, 1);
                    }
                    break;

                case VK_ESCAPE:
                    goto exit_loop;  //goto跳转语句
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
//跳转至退出程序
exit_loop:
    //1.停止后台绘制
    EndBatchDraw();
    //2.停止音频播放
    stopCurrentSong();
    //3.保存当前歌单
    savePlaylistToFile(&playlist, "playlist.txt");
    //逆序释放动态内存
    freePlaylist(&playlist);
    //删除UI元素[界面元素]
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
    //关闭系统资源
    closegraph();
    //显示退出信息
    printf("// 我想你可能想说拜拜！Ciallo~"); 
    return 0;
}
