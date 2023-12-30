使用方法(server, client)
1. git clone此repo

2. 進入此資料夾

3. 第一次使用請先在linux terminal中輸入make install指令下載所需套件

4. 編譯請使用在linux terminal中輸入make指令

5. 請準備總共三個terminal在此資料夾下，分別為server、client1、client2的

6. server視窗中輸入./server開啟server

7. 兩個client視窗中輸入./client 127.0.0.1 12345
  (在本地的話地址是127.0.0.1，同一個內網不同設備請輸入server的ip位置
  
8. 即可在兩個client端中傳輸訊息

使用方法(test_ncurses)
1. make ncurses 編譯

2. ./test_ncurses 執行

debug

- ERROR on binding: Address already in use:
  - 更改port即可，因為程式結束該port可能不會馬上被電腦free掉
- make: *** No rule to make target 'install'.  Stop.
  - 把Makefile的副檔名刪除
