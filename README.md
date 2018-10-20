# 0x6666
__This repo is about some essential techniques in programming__

------

### Data Structure

 - Intrusive Heap
 	- base on linked-list.
		- 支持erase.
	- base on vector
		- random data性能接近stl::heap, items with same value性能吊打stl.
		- 支持erase.

 - Intrusive Skiplist base.

 - Blocking Queue
 	- 没什么特点，双条件变量 + mutex

 - Tools
 	- Lock 统一了独占锁和共享锁的接口
		- mutex（互斥锁） 直接对std::mutex的封装
		- shared\_lock（多读一写） 在互斥锁的基础上配合atomic实现,性能爆表

 - Unit test
	- 用于单元测试
