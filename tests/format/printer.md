# printer

`#` 这一节不收集。下面每节是一条用例。`indent` 是缩进级数。`color` 为 `on` 时传入 `color_map()`，为 `mock` 时传入 `mock_color_map()`，为 `off` 时传入 `color_off_map()`。其余环境变量是 `$name$` 的替换表。

## Literal @htest.suite@ `Literal`

```text @htest.label@ input
hello
```

```text @htest.label@ output
hello
```

- @htest.match@ ordered
  - @htest.input@ !label input
  - @htest.expect@ !label output

## Name @htest.suite@ `Name`

- @htest.env@ !set name Bob

```text @htest.label@ input
Hi $name$.
```

```text @htest.label@ output
Hi Bob.
```

- @htest.match@ ordered
  - @htest.input@ !label input
  - @htest.expect@ !label output

## Indent @htest.suite@ `Indent`

- @htest.env@ !set indent 1

```text @htest.label@ input
one
two
```

```text @htest.label@ output
  one
  two
```

- @htest.match@ ordered
  - @htest.input@ !label input
  - @htest.expect@ !label output

## ColorOff @htest.suite@ `ColorOff`

- @htest.env@ !set color off

```text @htest.label@ input
$RED$bad$RESET$
```

```text @htest.label@ output
bad
```

- @htest.match@ ordered
  - @htest.input@ !label input
  - @htest.expect@ !label output

## ColorOn @htest.suite@ `ColorOn`

- @htest.env@ !set color on

```text @htest.label@ input
$RED$bad$GREEN$ok$YELLOW$y$BLUE$b$MAGENTA$m$CYAN$c$WHITE$w$DEFAULT$d$RESET$
```

```text @htest.label@ output
[0;31mbad[0;32mok[0;33my[0;34mb[0;35mm[0;36mc[0;37mw[0md[0m
```

- @htest.match@ ordered
  - @htest.input@ !label input
  - @htest.expect@ !label output

## ColorMock @htest.suite@ `ColorMock`

- @htest.env@ !set color mock

```text @htest.label@ input
$RED$bad$GREEN$ok$YELLOW$y$BLUE$b$MAGENTA$m$CYAN$c$WHITE$w$DEFAULT$d$RESET$
```

```text @htest.label@ output
<RED>bad<GREEN>ok<YELLOW>y<BLUE>b<MAGENTA>m<CYAN>c<WHITE>w<DEFAULT>d<RESET>
```

- @htest.match@ ordered
  - @htest.input@ !label input
  - @htest.expect@ !label output
