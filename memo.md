server.c: TCPのdaemon.cをリファクタしてみた  
client.c: TCPのclientほぼそのままなのに動作が重い
→ sleep時間が違うから。本家のusleep(10000)は10000 usec = 10 msec.こっちは100000000nanosec = 100000 usec = 100 msec.

serverは子プロセスが死なない。ので接続するたびに子プロセスが増える  
→ 子プロセス側でwaitしても意味がない。子プロセスはcloseしてreturnで死ぬのが仕事。そしたらdefunctになる。自分をwaitしたらそら永遠に終わらんわな

TCPのほうは^Aでちゃんとdefunctになるのに(しっかりkillされるのは別の接続が発生したとき)
→ 子プロセスができた瞬間、親プロセスは continue してacceptでblockし続けているから。`terminate_exited_processes` にたどり着くのはacceptが来てまたcontinueしてから。

とりあえず元の振る舞いと同じにできた  
clientはもっとリファクタできる
