# mark 设计稿

用例写在 markdown 里，二进制放在旁边的文件。样例见 `mark/grammar.md` 与 `mark/data.bin`。

## 文件与标题

- 只解析 ATX 的 `##` 和 `###`，并且 `setext == false`。cmark 里一行 `-` 也是 2 级标题，那种下划线不算。
- `##` 是一个独立测试单元，单独起一个进程，相当于一个带 `main` 的测试文件。这一节的 `@htest.flag@` 和 htest 启动参数在这个进程的入口生效。同一个进程里中途改标志，启动时已经读过的参数不会再生效。它的 suite 名写在 `@htest.suite@` 后面的行内代码里。
- `###` 是这个 `##` 里面的子测试，在所属 `##` 的进程里跑，不再另起进程。suite 名写在行内代码里，全名带上所属 `##` 的前缀。不在任何 `##` 下的 `###` 不合法。
- `#`、`####` 以及更深的标题不解析。它们是注释或文档，整段忽略。这些级别上不得出现 `@htest.suite@`；出现了也不收集、不报错。

## 写法

列表项是 `- @htest.<族>.<名>@` 加若干行内代码。反引号包几层都行，片段内容原样保留，包括 `=` 和空格。

`!` 只放在命令前。合法命令是 `!set`、`!drop`、`!load`、`!label`。没有 `!` 的命令不合法。`argv`、参数、flag、`@htest.expose@` 不加 `!`。

环境变量的索引方式见「Env」。进程里看到的是去掉 suite 前缀之后的 `key=value`。

外部字节用链接，路径在链接的 `url` 上，相对该 markdown 所在目录。谁可以引用这块数据，见下一节。

## Label manager

不用文件名当命名空间。标签的全局名字是从外到内的 suite 名，再加局部名，用 `.` 接起来。加载完所有 markdown 之后，管理器按这个全名索引。运行时按全名查找，不再打开文件。

只有 `##` 和 `###` 可以声明 `@htest.suite@`。`##` 是沙箱，`###` 是它的子测试。全名只由这两级从外往里拼。

```markdown
## Identity pass @htest.suite@ `Identity-pass`

```text @htest.label@ input1
alpha
beta
```

```text @htest.label@ output1
alpha
beta
```

### Identity pass sub @htest.suite@ `subcase1`

```text @htest.label@ output1
alpha
beta
```
```

上面三个标签的全名是 `Identity-pass.input1`、`Identity-pass.output1`、`Identity-pass.subcase1.output1`。

当前 suite 里用 `!label` 加局部名，管理器补上当前 suite 前缀。别的 suite 的标签用 `!load` 加全名。全名重复，加载失败。

三种登记方式：

围栏定义一块正文。信息行是「格式、`@htest.label@`、局部名」。格式决定怎么理解正文，例如 `text` 按字面，`json` 按 JSON 语义。登记到当前 suite 前缀下。

```markdown
```text @htest.label@ input1
alpha
beta
```

```json @htest.label@ expect1
{ "key": "value", "int": 123 }
```
```

`@htest.expose@` 把一个数据文件导入并挂到一个标签上。它出现在哪个 suite 章节里，全名就用哪一节的前缀，不另算一层。链接文字只是说明，路径在 `url` 里。

写在 `Identity-pass` 下的 `label1`，全名是 `Identity-pass.label1`。写在它的子标题 `subcase1` 下，全名是 `Identity-pass.subcase1.label1`。别的章节用 `!load` 加这个全名来引用。

```markdown
# shared @htest.suite@
- @htest.expose@ label1 [packet bytes](data.bin)
```

`!load` 也登记一个标签，并把它作为当前这一侧的数据。

```markdown
- @htest.input@ !load label1 [packet bytes](data.bin)
```

引用必须带动词：

- `!label output1` 只引用本节（当前 suite）的标签。在子 suite 里，它的全名是 `Identity-pass.subcase1.output1`。
- `!load` 后面加全名的行内代码，引用别的 suite 里已经登记的标签。例如加载 `Identity-pass.subcase1.output1`。
- `!load label1 [说明](data.bin)` 从文件装入，并登记到当前 suite 前缀下。

没有动词的 `@htest.input@ input1` 不合法。同一 suite 里，围栏或 `!load` 的局部名与 expose 重名时，`!label` 用本用例里的那个。

```markdown
- @htest.match.ordered@
  - @htest.input@ !label label1
  - @htest.expect@ !label expect1
```

## Env

环境变量存在一个 `std::map<string, string>` 里。map 的 key 是全名，和 label 同一套 suite 前缀，不含沙箱那一层：`Identity-pass.KEY`，子测试里是 `Identity-pass.subcase1.KEY`。`std::map` 按字典序排，同一前缀的条目是连续的一段，用 `lower_bound(前缀)` 做到这段的开头。

markdown 里只写局部名和值：

```markdown
- @htest.env@ !set KEY value
- @htest.env@ !drop KEY
```

`!set` 写在哪一节，就登记在哪一节的前缀下。`!drop` 只删这一节的全名，不删父级的同名 key。

用的时候做前缀匹配，再把前缀拿掉，叠成进程里真正的环境：

- 先取 `Identity-pass.` 这一段，得到 `KEY`
- 再取 `Identity-pass.subcase1.` 这一段，同名局部 key 盖掉父级
- 兄弟 `###` 各看各的前缀，互不可见

进程里只看见 `KEY=value`，看不见 suite 前缀。

## 数据

| 标记 | 参数 | 作用 |
|------|------|------|
| `@htest.input@` | 内联名、链接，或 `!load` | 本组喂给被测代码的字节 |
| `@htest.expected@` | 同上 | 本组期望 |
| `@htest.env@` | `!set` / `!drop` | 写当前 suite 前缀下的环境变量，见「Env」 |
| `@htest.argv@` | 每项一组行内代码 | 被测程序的启动参数，收成 `vector<string>`。至少一项。不在 CMake 或脚本里为每个用例再适配 |
| `@htest.flag@` | `` `名字` `值` `` | 本用例执行期间的 htest 标志，名字不带 `htest_` 前缀。进入前保存，结束后恢复 |
| `@htest.fixture@` | 已登记的夹具名 | 可选。没有则做内置的输入/期望比较。有则按用例新建，结束销毁 |
| `@htest.fixture.param@` | `` `键` `值` `` | 本用例的 `map<string, string>`。必须正好两段。空键、重复键不合法。没有 fixture 却写了参数，不合法。缺省值由夹具补 |

`@htest.suite@` 里的 `fixture.param` 和 `flag` 是全文件默认值，用例里的同名项盖掉它们。

`@htest.flag@` 不接受不认识的名字、空值、`filter`、`internal_run_death_test`。跑哪些用例由运行器过滤，不写进 flag，也不写进 argv。

夹具模板只能选 C++ 已经登记并编译进来的类型，例如 `fixture.param` 的 `T` = `int`。markdown 不能造出新的 `T`。实参仍是字符串，由夹具按 `T` 解析。

## 比较

`input`、`expected`、`compare.*` 是一组。一个用例里可以有多组，按各自出现的顺序配对：第 n 个 input、第 n 个 expected、第 n 个 compare 是同一次比较。三个标记的个数必须相同，否则加载失败。

模式写在标记名里，不再写 `@htest.compare@ ordered`。

| 标记 | 作用 |
|------|------|
| `@htest.compare.ordered@` | 按行顺序比 |
| `@htest.compare.set@` | 按集合比 |
| `@htest.compare.multiset@` | 按多重集合比 |
| `@htest.compare.subset@` | 期望是实际的子集 |
| `@htest.compare.regex@` | 期望的每一行是正则，匹配实际的对应行 |
| `@htest.compare.approx@` | 按数值比，容差用参数 `epsilon` |

二进制结构体不拿原始字节当期望文本。结构体作者用 fmt 的 `formatter<T>` 按语义打出文本，期望和实际都走这份文本。不用 `PrintTo`。

fmt 内置进 htest。默认用内置副本。用户已有 fmt 时打开显式开关（如 `HAWKTEST_FMT_EXTERNAL`），改为使用外部头文件，并且不再编译内置源文件。不在编译期自动探测，避免两套 fmt 同时进二进制。

## 用例控制

| 标记 | 参数 | 作用 |
|------|------|------|
| `@htest.case.skip@` | 原因 | 收集但不跑 |
| `@htest.case.xfail@` | 原因 | 失败算过，通过则报「不该过」 |
| `@htest.case.timeout@` | 如 `` `2s` `` | 到点终止，算失败 |
| `@htest.case.until@` | 如 `` `5s` `` | 时限内反复比较，通过即停 |

`@htest.case.xfail@`、`@htest.case.timeout@`、`@htest.case.until@` 这一期没做，下一步再做。

## mock

匹配器和调用都在 C++ 里登记。markdown 只点名字、给参数，不写 C++ 表达式。

| 标记 | 参数 | 作用 |
|------|------|------|
| `@htest.mock.death@` | `` `exited` `码` `正则` `` 或 `` `signaled` `信号` `正则` `` | 在子进程里跑。输出按 `ContainsRegex` 匹配 |
| `@htest.mock.throws@` | 已登记的异常类型名 | 当前进程必须抛出该类型 |
| `@htest.mock.expect@` | `` `字段` `匹配器` `参数…` `` | `EXPECT_THAT`。一条用例可以有多条 |
| `@htest.mock.call@` | `` `方法` `次数` `` | 夹具上的期望调用。次数如 `once`、`never`、`times 3` |

`@htest.mock.death@` 与 `@htest.mock.throws@` 不能同时出现。没有 fixture 时，`mock.call` 不合法。

`@htest.mock.death@`、`@htest.mock.throws@`、`@htest.mock.expect@`、`@htest.mock.call@` 这一期没做，下一步再做。

## flag

markdown 里的名字和原来的 flag 同名，例如 `death_test_style`、`color`、`repeat`。不加 `htest_` 前缀，也不另起别名。`filter` 和 `internal_run_death_test` 拒绝。设置时改对应的 `FLAGS_htest_*`，用例结束按进入前的值恢复。

## 还没定

- `@htest.mock.call@` 的返回字节用哪一个标记承接。
