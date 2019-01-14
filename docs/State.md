# State

An entity that manages transitioning and exposing state to `view`'s.

## Type Signature

```
state Counter($value = 0) {
  mut increment -> $value++;
  mut decrement -> $value--;
}
```

## Constructing state containers

Parameters provided to `state` containers witho no default values must be provided
to use the state in a provider.

```
state Counter(value: number) { }
```

### Exposing constructor values

The value of `value` will be made available as `$value` or on the alias directly as `value`.

```
state Counter($value: number) { }
```

### Default constructor values

Constructor arguments can have default values provided.

```
state Counter(value = 0) { }
```

## Mutate state values

Modify the values in your state purely with mutators.

```
state StateWithMutator {
  value = 0;

  mut add(addend: number) -> $value += addend;
}
```

## Private state values

Simplify the interface of your state container by hiding internal values.

```
state PrivateState {
  private quarters: number;
  private dimes: number;
  private nickels: number;
}
```

## Getters to override or expose values

```
state StateWithGetter {
  private quarters: number;
  private dimes: number;
  private nickels: number;

  get total(pennies: number) -> $quarters * 25 + $dimes * 10 + $nickels * 5 + pennies;
}
```

## Omit Parentheses

When your signature or getter or mutator does not accept parameters, the parentheses can be omitted.

```
// not preferred
state SimpleState() {
  get total() -> /* ... */;

  mut increment() -> /* ... */;
}

// preferred
state SimpleState {
  get total -> /* ... */;

  mut increment -> /* ... */;
}
```
