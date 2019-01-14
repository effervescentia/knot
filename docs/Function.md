# Function

Use functions to modularize data extraction, generation, effect dispatch and object mutation.

## Definition

Use the `func` keyword to define either inline or multi-line functions.

```
func increment(value: number) -> value++;

func addTo10(lhs: number, rhs: number) -> {
  let total = lhs + rhs;

  if (total > 10) {
    return 10;
  } else {
    return total;
  }
}

func getSum(lhs: number, rhs: number): number -> {
  return 'abc'; // will not compile
}
```

## Argument Order

Arguments in `knot` can be matched through punning rather than by position.
Use the parameter explosion function call modifier (`#`) to use named arguments and enable punning.

```
func addTogether(lhs: number, rhs: number) -> lhs + rhs;

addTogether(20, 4); // 24

let rhs = 4;

addTogether(30, rhs: 40); // ERROR: cannot mix positional arguments with named arguments
addTogether#(30, rhs: 40); // ERROR: must name each argument when using explosion modifier
```
