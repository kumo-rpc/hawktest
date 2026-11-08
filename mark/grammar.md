# 说明

这一节是文档。`#` 不解析，里面的 `@htest` 也不收集。

# 沙箱 @htest.sandbox@

这一节单独起进程。`argv` 只放在这里，作为那个进程的启动参数。

- @htest.argv@ `identity` `--input` `packet`

## Identity pass @htest.suite@ `Identity-pass`

- @htest.fixture@ `Demo`
- @htest.fixture.param@ `count` `2`
- @htest.flag@ `death_test_style` `threadsafe`
- @htest.env@ !set KEY alpha
- @htest.expose@ packet [packet bytes](data.bin)

```text @htest.label@ input1
alpha
beta
```

```text @htest.label@ output1
alpha
beta
```

```json @htest.label@ expect1
{
  "key": "value",
  "int": 123
}
```

- @htest.match@ ordered
  - @htest.input@ !label input1
  - @htest.expect@ !label output1

```text @htest.label@ pattern1
alpha
b.*
```

- @htest.match@ ordered
  - @htest.input@ !label packet
  - @htest.expect@ !load packet [packet bytes](data.bin)

- @htest.match@ regex
  - @htest.input@ !label input1
  - @htest.expect@ !label pattern1

### 子测试 @htest.suite@ `subcase1`

本节前缀是 `Identity-pass.subcase1`。父级的 `KEY` 仍在，这里把它删掉再设成本节自己的值。

- @htest.env@ !drop KEY
- @htest.env@ !set KEY beta

```text @htest.label@ output1
beta
```

- @htest.match@ ordered
  - @htest.input@ !load `Identity-pass.input1`
  - @htest.expect@ !load `Identity-pass.output1`

- @htest.match@ approx
  - @htest.input@ !load `Identity-pass.expect1`
  - @htest.expect@ !load `Identity-pass.expect1`

#### 更深层的标题

`####` 不解析。下面这段不是用例。

- @htest.env@ !set IGNORED 1

```text @htest.label@ ignored
nope
```

## Mismatch @htest.suite@ `Mismatch`

这条用例故意对不上，用来看失败输出。

```text @htest.label@ input1
alpha
beta
```

```text @htest.label@ output1
alpha
gamma
```

- @htest.match@ ordered
  - @htest.input@ !label input1
  - @htest.expect@ !label output1
