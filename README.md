
FastKEngine is a knowledge engine based keywords matching.

* Format

```
[[base id=###]]

[[question id=###]]
  question1
  question2
  ...

[[answer key1=value1 ...]]
    the answer can be multi-lines

```

* Notice

```
  the charset is UTF-8.

  multi questions correspond to one answer, question matchs success
  when all required keywords of a question are matched.

  you should segment Chinese words manually.

  the conditions like "key1=value1 key2=value2" in the answer is optional,
  eg. [[answer]] for no additional condition. the key and value can be quoted with
  double or single quotation marks. eg. [[answer uname="Linux"]]. value can be a
  function like FUNC(param1, param2, ...), support functions:
    IN(), such as: IN(Darwin,FreeBSD)

  the answer is composite when [[answer ...]] occurs many times after one [[question]].

  question keywords:
    * with a pipe line (|) for match any one of the keywords, eg.
      (core dump) location|position
      core-dump (location | position)

      hint: the second question use round brackets to ignore spaces with pipe line (|)

    * keywords quoted within  [ and ] are optional keywords, [...] must be
      the last part of the question. eg.
      file size descending [list | show]

    * multi English words as a matching keyword when:
        I.  minus sign (-) between multi words, such as core-dump, out-of-memory
        II. quoted with round brackets, such as (core dump), (out of memory）
      eg. "core-dump" matchs "core dump", "coredump" and "core-dump", but NOT match "dump core".

    * config file similars.txt for similar keywords which split by space or tab, such as:
      set setting config configure configuration
      OOM (out of memory) (out memory)
```

* Example

```

[[base id=1101000000]]

[[question id=123]]
  如何 生成 core-dump
  core-dump 设置

[[answer]]
 1. ulimit -c 查看内核core dump文件的大小限制，输出为0表示不能生成core dump文件，此时需要进行如下设置：
 ulimit -c unlimited 

 [[answer uname=Linux]]
 如果ulimit -c unlimited  执行失败，需要设置系统文件/etc/security/limits.conf，注释掉如下行即可：
  *   soft    core            0
  *   hard    core            100000

 2. 通过sysctl检查和设置内核参数：
 kernel.core_pattern: 生成的core dump文件名，最好包含全路径
 fs.suid_dumpable：设置为1或2

 检查命令：
 [[cmd required/]] sysctl kernel.core_pattern fs.suid_dumpable

 设置命令：
 [[cmd required]]
 sudo sysctl -w kernel.core_pattern=/tmp/core.%p
 sudo sysctl -w fs.suid_dumpable=1
 [[/cmd]]

 [[answer uname=Darwin]]
 2. 通过sysctl检查和设置内核参数：
 kern.corefile: 生成的core dump文件名，最好包含全路径，例如：/cores/core.%P
 kern.coredump：设置为1
 kern.sugid_coredump：设置为1

 检查命令：
 [[cmd required/]] sysctl kern.corefile kern.coredump kern.sugid_coredump

 设置命令：
 [[cmd required]]
 sudo sysctl -w kern.corefile=/cores/core.%P
 sudo sysctl -w kern.coredump=1
 sudo sysctl -w kern.sugid_coredump=1
 [[/cmd]]


[[question id=124]]
  core-dump 位置

[[answer]]
 执行命令：

 [[answer uname=Linux]]
 [[cmd required/]] sysctl kernel.core_pattern

 [[answer uname=Darwin]]
 [[cmd required/]] sysctl kern.corefile

[[answer]]
  输出的是core dump文件位置


[[question id=125]]
  时间 降序 文件|目录|显示|列举|ls

[[answer]]
 ls 带上参数-t即可，例如：
 [[cmd required/]] ls -lt [filename]


[[question id=126]]
  时间 升序 文件|目录|显示|列举|ls

[[answer]]
 ls 带上参数-rt即可，例如：
 [[cmd required/]] ls -lrt [filename]

[[question id=127]]
  文件|目录|ls 大小 降序 [显示|列举]

[[answer]]
 ls 带上参数-S即可，例如：
 [[cmd required/]] ls -lS [filename]


[[question id=128]]
  文件|目录|ls 大小 升序 [显示|列举]

[[answer]]
 ls 带上参数-rS即可，例如：
 [[cmd required/]] ls -lrS [filename]


[[question id=129]]
  显示|列举 文件|目录
  ls

[[answer]]
 ls用于列举文件和目录，常用命令:
 显示详细文件列表：
 [[cmd/]] ls -l [filename]

 人性化地显示详细文件列表（文件大小显示非常直观）：
 [[cmd/]] ls -lh [filename]

 每行只列出一个文件/目录名（shell脚本中常用做法）：
 [[cmd/]] ls -1 [filename]

 [[answer uname=Linux]]
 ll 命令等价于 ls -l


[[question id=130]]
  删除 文件|目录
  rm

[[answer]]
 rm用于删除文件和目录，rm带上-r参数可以删除目录。常用命令:
 不询问删除文件：
 [[cmd/]] rm -f <filename>

 删除目录和子目录以及目录下的所有文件：
 [[cmd/]] rm -r <filepath>

 注意：文件和目录删除后不可恢复，请谨慎使用以上命令，尤其是rm -rf
       rm -rf 后面的filepath最好明确指定路径名称，尽量不要使用 *
       切勿尝试执行：rm -rf /


[[question id=131]]
  删除 空 目录
  rmdir

[[answer]]
 rmdir用于删除空目录，命令示例：
 [[cmd required/]] rmdir <filepath>

```

```
上述知识库示例中对应的近义词列表：

文件 文档 file
目录 directory dir
时间 日期 time date datetime
显示 展示 show
列举 列出 排列 list
降序 倒序 倒排 descending descend desc
升序 正序 顺序 ascending ascend asc
如何 怎样 how how-to
生成 产生 产出 形成 generate produce
大小 尺寸 size
位置 地方 哪儿 哪里 那儿 那里 position  location place
设置 设定 配置 set setting config configure configuration
删除 移除 delete remove
```
