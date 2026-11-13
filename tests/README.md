# Focused regression tests

Run from the repository root with a C++17 compiler:

```sh
./tests/run.sh
CXXFLAGS='-g -fsanitize=address,undefined -fno-omit-frame-pointer' ./tests/run.sh
```

The repository previously had no test framework, build files, or test conventions.
These tests use standard C++ assertions without adding dependencies. `CXX` and
`CXXFLAGS` can override the compiler and flags; assertions remain enabled even
when the supplied flags define `NDEBUG`. Build artifacts are temporary.

## Risk assessment and scope

The repository contains memory-reading snippets rather than application code;
there are no payment, authentication, or persistent-storage implementations.
The highest-risk input-handling path is `FName.h`: external memory supplies
addresses, redirect indices, length headers, and encrypted string bytes.

The five tests exercise the actual header with strict, ordered in-memory read
expectations. Unexpected addresses, read sizes, extra reads, and missing reads
fail the tests. Coverage includes:

- Direct names with nonzero chunk/entry indices and low header flag bits.
- Redirects at the header boundary (63), with independent redirected indices.
- Empty redirected names, including a zero-byte payload read.
- The minimum direct header (64), a single character, and string termination.
- The maximum 16-bit header (65535): exactly 1023 bytes, a complete decoded
  string, and termination. The 2048-byte clamp cannot be reached by this header.

Short ciphertext fixtures are fixed known answers; the maximum-length fixture
uses an inverse encoder to exercise the full buffer without a large data file.

Other untested risks remain in `Camera.h` (external rotation/FOV data),
`GetBoneWithRotation.h` (pointer fallback and index arithmetic), and `va_text.h`
(address scanning). These depend on host-provided types/readers; the camera file
also contains two alternative definitions of the same function. This suite does
not validate live memory access, driver behavior, invalid pointers/read failures,
or those other snippets. No production code is changed.
