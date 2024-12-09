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

**Equality Queries**

On the hardcoded version, an implementation of CountMinSketch (for A=x, B=x) and a BloomFilter (for A=x AND B=y) were tested.
CMS works as good as return 1 (which is the average on a uniform distribution, 20mil tuples / 20mil value range)
BF is worse than return 0, which makes sense, because (A=x AND B=y) for a *random* value is highly improbable, and most likely zero.
We will test those two techniques in the future, using customized datasets, that contain tuples and respective queries that satisfy those parameters.
There are testcases like that in the competition's evaluation system, almost certainly.

For now, queries like A=x AND A>y, are mostly hopeless against zero, but we'll figure it out later.

**Hardcoded Datasets**

We must tidy up the hardcoded branch a bit.
We must create some realistic hardcoded datasets (60mil tuples, 200k queries, no insert/delete). (AWS will be utilized)
At least two with random (uniform) values, and two that will overrepresent cases A=x, B=x, A=x AND B=y, to test CMS & BF against zero.

**Execution time improvement**

After various failed and successful submissions, we have finally isolated the cost of our current estimators, which
mostly work well for queries with only Greater comparison (A>x, B>x, A>x AND B>y).
Our runtime is off limits. In order to test the current implementation (histogram & frequency arrays), we have to
create multiple copies with half the size.
This only increases the insertion by a little (from O(3) to about O(15)) and improves traversal logarithmically, from 270k to 18 traversals per query.
This is our next best step, to submit something that does actual computations and produces a better score.
---
