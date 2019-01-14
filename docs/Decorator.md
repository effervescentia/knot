# Decorator

Decorators can be used to modify the behaviour of `view`'s and `state`'s.

```
state MyState {
  x: number;
  y: number;
}

type

func withState()

@require(MyState)
view MyView {

}
```
