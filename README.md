[![Build & Test](https://github.com/codethinki/cth/actions/workflows/build-test.yml/badge.svg)](https://github.com/codethinki/cth/actions/workflows/build-test.yml)

# cth
c++26 utility library

built and tested with:
- linux: gcc 16, clang 23
- windows: msvc, clang 23

# overview

## cth::cth
header only core

### data structures & allocators (`cth::dt`)
- `miniram`: O(1) offset allocator with defragmentation, fully constexpr (inspired by Sebastian Aaltonen's [OffsetAllocator](https://github.com/sebbbi/OffsetAllocator))
- `ring_alloc`: lock-free ring allocator, contiguous allocations
- `poly_vector`: struct of arrays in a single allocation
- `union_find`: disjoint sets on `poly_vector`
- `optional`: supports references and `void`, monadic operations
- `pool`, `thread_pool`, `string_joiner`

### concurrency primitives (`cth::co`)
- `atomic_fence`: manual reset event on `std::atomic` wait / notify, `atomic_heap_fence` as movable variant
- `global_switch`: thread safe, reference counted on / off switch for global state
- `unique_cohandle`: owning `std::coroutine_handle`
- `extract_awaiter`: the awaiter of any awaitable, member or free `operator co_await`

### metaprogramming (`cth::mta`)
- concepts as values: `CPT(std::integral)`
- multi dimensional range traits: `md_range_value_t`, `range2d_over_cpt`
- trait packs, tuple & variant helpers

### logging & exceptions (`cth::log`, `cth::except`)
- `CTH_STABLE_*` macros always active, `CTH_*` debug only
- capture `std::source_location` and `std::stacktrace`
- trailing block runs only if the condition holds

### formatting
- `CTH_FORMAT_CLASS`, `CTH_FORMAT_ALIAS`, `CTH_FORMAT_CPT`: generate `std::formatter` specializations for classes, wrapper types and all types of a concept

### algorithms (`cth::alg`, `cth::views`)
- `unique_combine` / `assign`: bipartite matching on 2d ranges
- pipeable views: `split_into`, `drop_stride`, `transform_call`, `to_ptr_range`

### hashing (`cth::hash`)
- `combine`, range hashing
- `hash_aggregate`: hashes aggregates member wise

### pointers
- `not_null`, `move_ptr`

### io (`cth::io`)
- colored console output: `col_stream`
- file reading: `file::chop`, `file::read`

## cth::coro
coroutine runtime on asio

- `co::scheduler`: `asio::io_context` on its own threads
- `co::executor`: spawns tasks, moves execution back onto the scheduler after foreign awaitables
- `this_coro::executor`, `this_coro::wait(duration)`, native handle waits

## cth::os
portable waitable handles

- native handles: fd (linux), `HANDLE` (windows)
- `os::fence`: eventfd (linux), event (windows)
- `os::timer`: timerfd (linux), high resolution waitable timer (windows), sub millisecond waits (asio's `steady_timer` can't)

## cth::win
windows only

- win32 wrappers without leaking `<Windows.h>`
- `cth::win::capture`: window capture

# examples

## offset allocation & defragmentation
```cpp
cth::dt::miniram ram{1024};

auto const a = ram.allocate(256);
auto const b = ram.allocate(256);
ram.free(a);

auto const report = ram.defragment(); // b moves to offset 0
for(auto const& move : report.moves)
    std::memmove(memory + move.dstOffset, memory + move.srcOffset, move.size);
```

## coroutines
```cpp
cth::co::scheduler scheduler{cth::co::autostart, 2};
cth::co::executor executor{scheduler};

auto task = []() -> cth::co::executor_task<int> {
    co_await cth::co::this_coro::wait(std::chrono::microseconds{200});
    co_return 42;
};

int const result = cth::co::sync(executor.spawn(task()));
```

## logging
from `cth/io/file.hpp`:
```cpp
CTH_STABLE_ERR(!file.is_open(), "failed to open file") {
    details->add("file: {0}", path.string());
    throw details->exception();
}
```

# building

## requirements
- cmake 4.1+, ninja
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set
- gcc 16, clang 22+ (libstdc++) or msvc

## build & test
```bash
git clone --recursive https://github.com/codethinki/cth.git
cd cth
cmake --preset gcc_release
cmake --build out/build/gcc_release
ctest --test-dir out/build/gcc_release
```

## presets
- `gcc_release`, `clang_release`, `msvc_release`
- windows only: `clang_debug`, `msvc_debug`

# using cth in another project
building installs the package into `out/install/<preset>` (target `cth_package`), point `cth_ROOT` there:

```cmake
find_package(cth CONFIG REQUIRED COMPONENTS coro)
target_link_libraries(app PRIVATE cth::coro)
```

api not stable yet, no versioned releases

## components
- `cth`, `os`, `coro`
- windows only: `win`, `win_capture`

# license
[MIT](LICENSE)
