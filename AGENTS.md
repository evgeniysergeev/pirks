# Agent Guidelines

## C++ readability

Prefer simple code over mechanically enforcing a single-return shape. Use a
linear success path with the main return at the end when it stays clear, and use
early returns or throws for error, empty, or invalid cases.
For candidate-search loops, it is fine to throw after the loop when no match
was found if that keeps the code simpler.

For functions returning containers, prefer `return {};` for early error exits
before any values have been collected. Declare result containers close to the
loop or block that fills them.

Use trailing underscores only for non-static class data members. Do not use
trailing underscores for local variables, parameters, or temporary values; pick
a more specific local name when needed.

Use prefix increment/decrement in `for` loop headers, e.g. `++i` or `--i`, for
consistency across numeric counters and iterators. Use postfix
increment/decrement, e.g. `count++` or `count--`, for ordinary arithmetic
updates outside loop headers when the old value is not used.
