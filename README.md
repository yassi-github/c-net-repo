# C-NET-REPO

[![Tests](https://github.com/yassi-github/c-net-repo/actions/workflows/test.yaml/badge.svg)](https://github.com/yassi-github/c-net-repo/actions/workflows/test.yaml)


## Dev Requirements

- [googletest](https://github.com/google/googletest)
- [clang-format](https://releases.llvm.org/16.0.0/tools/clang/docs/ClangFormat.html)

## tips

delete all message queues:

```shell
# show current status
ipcs
# delete all msg queues
ipcrm --all=msg
```
