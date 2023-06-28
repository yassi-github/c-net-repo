server.c: TCPのdaemon.cをリファクタしてみた  
client.c: TCPのclientほぼそのままなのに動作が重い

serverは子プロセスが死なない。ので接続するたびに子プロセスが増える  
TCPのほうは^Aでちゃんとdefunctになるのに(しっかりkillされるのは別の接続が発生したとき)
