# Type

Used to represent the type of a keyed object or a function.

## Keyed Object Type

An object with keys can be represented as a `type`.

```
type SomeObject = {
  name: string;
  age: number;
  loggedIn: boolean;
};
```

## Function Type

```
type ObjectTransformer = (value: string) -> string;
```

### Positional arguments only

Types delcared this way cannot be called with the parameter explosion modifier (`#`).

```
type Adder = (number, number) -> number;
```

## State Types

Object types that can include getters and mutators.

```
type Counter = {
  value: number;

  get isGreater(otherValue: number): boolean;

  mut increment();
};
```

## View Types

Use the result of a view composition to specify a type, these are computed at compile time.

### Property interface

Describe the props that must be passed to the component on rendering.

```
type SomeView = View({
  name: string;
});
```

### Duck-typed state dependency interface

Must be provided with the mutators / getters / values in the specified `type`.

```
type SomeState = {
  value: number;

  mut increment();
};

type SomeView = View({}, [SomeState]);
```

### Typed state dependency interface with alias

For use with components that specify aliases for their providers.

```
state SomeState = { };

type SomeView = View({}, [SomeState as Counter]);
```

### Extend an existing view type

```
type BaseView = View({
  value: string;
});

type InheritingView = View({
  name: string;
}) & BaseView;
```

### Declaring state provider types

```
type SomeState = {
  value: string;
};

type BaseView = View({}, [], [SomeState]);

type BaseView = View({}, [], [SomeState as Other]);
```

### Declaring partially applied dependencies and providers

```
type SomeState = (a: number, b: number) -> { };
type SomeProvider = (c: number, d: number) -> { };

type BaseView = View({}, [(b: number) -> SomeState], [(c: number) -> SomeProvider]);
type InnerView = View({}, [SomeProvider]);

view SomeView(someView: BaseView, innerView: InnerView) {
  <SomeState b={13}>
    <someView>
      <SomeProvider d={123}>
        <innerView />
      </SomeProvider>
    </someView>
  </SomeState>
}
```

## View Composition Types

Views that allow parameters to be passed as the child of the JSX tag.
By default the child type is `any`.

```
type ComposableView = View({}, [], [], any);

// or

// view type constructors can be immediately partially applied since all arguments are optional
type ComposableView = View#(child: any);

view MyView {
  <div>
    <slot />
  </div>
}

// `MyView` matches type `ComposableView`
```

### Mapped View Composition Types

```
type MappedComposableView = View#(child: { a: View, b: View });

view MappedView {
  <div>
    <h1>
      <slot(a) />
    </h1>
    <p>
      <slot(b) />
    </p>
  </div>
}

// `MappedView` matches type `MappedComposableView`
```
