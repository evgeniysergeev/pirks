# Agent Guidelines

## C++ readability

For functions returning containers, prefer `return {};` for early error exits
before any values have been collected. Declare result containers close to the
loop or block that fills them.

Use trailing underscores only for non-static class data members. Do not use
trailing underscores for local variables, parameters, or temporary values; pick
a more specific local name when needed.
