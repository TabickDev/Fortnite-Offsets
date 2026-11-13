# Camera regression test

Run from the repository root with a C++20 compiler:

```sh
g++ -std=c++20 -Wall -Wextra -Werror -I. tests/camera_test.cpp -o /tmp/fortnite-camera-test
/tmp/fortnite-camera-test
```

The test supplies minimal camera types and typed, in-memory `read<T>` fixtures.
It includes the actual production header; no game process, driver, or Windows SDK
is required. It checks the controller's radians-to-degrees conversion for three
fields of view, camera location/rotation, and the existing local-player helper.

Before the fix, the header failed to compile because both variants were named
`get_view_point`. After distinguishing the controller variant, the regression
failed with `controller FOV in degrees: expected 60, got 0` because its conversion
result was discarded instead of assigned.
