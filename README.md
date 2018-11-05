# 0x6666
__This repo is about some essential techniques in programming__

------

### Data Structure

 - __Intrusive Heap__

      - base on linked-list.
          - 支持erase.
      - base on vector
          - random data性能接近stl::heap, items with same value性能吊打stl.
          - 支持erase.

 - __Intrusive Skiplist__

      - skiplist 普通侵入式跳表的实现
      - concurrent_skiplist 支持并发的侵入式跳表 ( 配合tools/lock )
          - TODO. 当前仅支持并发插入，下一步增加erase


__Performance__

__Base line: 2945ms__  12 vCPU in Alibaba-Cloud ECS _( insert 1048576 items into skiplist.)_

| threads |  concurrent skiplist | speedup ratio |
| :-----: |  :-----------------: | :---------: |
| 1       |  4312 ms |--|
| 2		  |  2497 ms|1.18|
| 4	| 1382 ms |2.12|
| 8	| 1002 ms |2.93|
| 12	| 749   ms |3.93|
       	

 - __Blocking Queue__

    - 没什么特点，双条件变量 + mutex

### Tools
  - Lock 统一了独占锁和共享锁的接口
    - mutex（互斥锁） 直接对std::mutex的封装
    - shared\_lock（多读一写） 在互斥锁的基础上配合atomic实现,性能爆表

###  Unit test
  用于单元测试
