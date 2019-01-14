# Constant

For representing unchanging values.

## Definition

Constants can be defined using the `const` keyword. Identifiers must be named in title case.

```
const RETRY_TIMEOUT = 1000;
```

### Reference

Constants can reference previously defined constants in its definition.

```
const BASE_TIMEOUT = 1000;
const EXTENDED_TIMEOUT = BASE_TIMEOUT * 2;
```
