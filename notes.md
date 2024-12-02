### Constraints

1. Space complexity: **4MB**
2. Time complexity : **10 seconds**
3. Queries: **20,000,000 queries** (max)
4. A, B values: [1, 20,000,000]

---

### Arithmetics:

MEM: 4MB = 4,194,304 Bytes = 33,554,432 bits
Each value in A, B requires 25 bits

---

### Histogram Algorithm:

**Questions**
1. How much memory per bucket?
2. How many Buckets?

**Answers**
1. Each bucket requires:  
    1.1 Bucket range boundaries (two integers, e.g., start and end)  
    1.2 Frequency count (integer)  
    1.3 Each bucket requires 12 bytes (96 bits)

2. Number of buckets:
    2.1 B = MEM / BucketSize = 4,194,304 bytes / 12 bytes = 349,526 buckets  

**Optimization**  
- Uniform Distribution: evenly sized buckets because all values are equally likely.
- Skewed Distribution : adaptive bucket sizes (e.g., more buckets in dense regions, fewer in sparse regions).

Histogram refinement (e.g., split buckets dynamically where skew is detected).
Compressed data structures (e.g., count-min sketch, HyperLogLog) if exact frequencies aren't required.

---
