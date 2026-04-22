# redis-from-temu

This is a recreation of a Redis-like KV cache for learning purposes.

It runs the same single-thread execution model as Redis, with persistency being maintained
using WAL strategy. Heavy `fsync` operations are offloaded to a background thread away from the core loop.