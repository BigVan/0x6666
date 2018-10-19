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

 - Unit test
 	- RW Lock
		- 基于普通mutex_lock和计数器
