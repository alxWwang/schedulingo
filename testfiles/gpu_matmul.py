"""GPU test job: repeatedly multiplies large matrices on the Apple GPU (via MLX).

Usage: python3 gpu_matmul.py <name> [iterations]
Prints how long its GPU work took, so you can compare jobs running alone vs. together.
"""
import sys
import time

import mlx.core as mx

name = sys.argv[1] if len(sys.argv) > 1 else "gpu"
iters = int(sys.argv[2]) if len(sys.argv) > 2 else 200

assert mx.default_device() == mx.gpu, "MLX is not using the GPU"

n = 4096
a = mx.random.normal((n, n))
b = mx.random.normal((n, n))
mx.eval(a, b)

start = time.perf_counter()
for _ in range(iters):
    c = a @ b
    mx.eval(c)          # MLX is lazy: eval forces the GPU to actually do the work
secs = time.perf_counter() - start

tflops = 2 * n**3 * iters / secs / 1e12
print(f"[{name}] {iters} matmuls of {n}x{n} on GPU: {secs:.2f}s ({tflops:.2f} TFLOPS)", flush=True)
