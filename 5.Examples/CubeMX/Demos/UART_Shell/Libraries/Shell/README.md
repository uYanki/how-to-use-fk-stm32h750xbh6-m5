### 命令导出

方式 ①

```c
#define CMD_EXPORT_FUNC(_attr, _name, _cbk, _desc)  \
    __SECTION("CMDS")                               \
    __USED static cmd_t _cmd_##_name = {                   \
        .attr.all  = _attr,                         \
        .func.name = #_name,                        \
        .func.cbk  = (cmd_cbk_t)_cbk,               \
        .func.desc = #_desc,                        \
    };
```

方式 ②

```c
#define CMD_EXPORT_FUNC(_attr, _name, _cbk, _desc)  \
    static const char _cmd_name_##_name[] = #_name; \
    static const char _cmd_desc_##_name[] = #_desc; \
    __SECTION("CMDS")                               \
    __USED static cmd_t _cmd_##_name = {                   \
        .attr.all  = _attr,                         \
        .func.name = _cmd_name_##_name,             \
        .func.cbk  = (cmd_cbk_t)_cbk,               \
        .func.desc = _cmd_desc_##_name,             \
    };
```

注1：方式①可能会被编译器优化掉，导致无法正常导出命令。

注2：不把 `cmd_t _cmd_##_name` 定义为 `const` 是因为要在 `shell_init()` 中计算命令的 `hash` 值，用于进行命令的快速匹配。

注3：命令导出的本质时使用 `__SECTION()` 指定变量放在相同的内存区域。

### 函数回调

方式 ①

```c
typedef int (*cmd_cbk_t)(shell_t*);
typedef int (*cmd_cbk_t)(void);
```

方式 ②

```c
typedef int (*cmd_cbk_t)();
```

注1：方式①把函数参数固定死了，在调用时只能传入指定的参数；方式②的参数留空，在调用时可以传入任意参数。

