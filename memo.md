server.c: TCPのdaemon.cをリファクタしてみた  
client.c: TCPのclient.cをリファクタしてみた

server: TCPのほうは^Aでちゃんとdefunctになるのに(しっかりkillされるのは別の接続が発生したとき)  
→ 子プロセスができた瞬間、親プロセスは continue してacceptでblockし続けているから。`terminate_exited_processes` にたどり着くのはacceptが来てまたcontinueしてから。

client: nonblock なので stdio 待ちが発生せず sleep が回り続けるので何もしなくても CPU 消費が激しい  
→ stdio送信まではブロックすべき？fcntlでホイホイ変えまくってもいいのか？

とりあえず元の振る舞いと同じにできた  

## 課題1

### ふるまい

client: messageをserverに送る
server: 受け取ったmessageから新たなmessageを作成し、ファイルに追記する(排他処理)

### 仕様

#### server

引数は `データ格納用ファイル名(XX)` と `ポート番号`

起動時に空の `XX` を作成、指定ポートでコネクション待ちになる。

clientから接続されると fork する。

親プロセス:
- connected socketを閉じる
- defunct子プロセスがあれば、waitpidですべて取り去る
- loopする

子プロセス:
- listening socketを閉じる
- 以下の排他処理を行う
    - `XX` を open
    - message を client に送る
        messageの内容は
        `XX`が空: `0 NONE NONE\0...`
        `XX`空でない: 最後のmessage
    - client からの message を待ち、届き次第`XX`に追記

(排他処理なしverも作る)

#### client

引数は `server_name` `id` `loop_num`  
なお`id` は 35 byte 以下とする

`loop_num`回だけ以下の処理をloopする:

- serverにTCP接続
- message 受け取る
- 受け取ったら表示してちょっと待つ
- 受け取ったのから新規メッセージ (`数字+1 myid recvid1\0...`) を作成してサーバに送る
- コネクションを切断
- ちょっと待つ

## 課題2

### ふるまい

親: 準備→子プロセス生成→loop{ じゃんけんの手入力受付,全子の手決定→結果表示 }
子: 手を決める乱数を初期化(リエントラントシード)→loop{ 親の指示待ち→手を決めて親に渡す }

- 終了条件を適当に決める
- 途中で子の追加と削除もできるバージョンもあり
    - キー入力で追加削除を行う

### 仕様

#### 親

init(): なんか準備
create_child(): 子プロセス生成
parent_janken_loop(): loop{ input_card(), fix_child_card(), show_card() }

#### 子

init_seed(): set seed
child_janken_loop(): loop{ wait_command(), rand_card(), pass_card_to_parent() }
