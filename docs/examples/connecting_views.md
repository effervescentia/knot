# Connecting Views

State can be connected to a view.

## Local State

State that shares the lifecycle of the attached component.

### States with constructor values

```
state MyState(initialValue: number) {
  value = initialValue;

  mut increment -> $value++;
}

view SomeView with MyState(8) {
  <div>
    <span>total: {$value}</span>
    <button onClick={$increment}>add</button>
  </div>
}
```

### States without constructor values or with default values

```
state DefaultedState(initialValue: number = 0) { }
state SimpleState() { }

view SomeView with DefaultedState, SimpleState {
  <div>
    <span>total: {$value}</span>
    <button onClick={$increment}>add</button>
  </div>
}
```

## Contextual State

Contextual state allows for components to communicate implicity through statically declared dependencies.

```
state DragManager(initialX: number, initialY: number) {
  x = initialX;
  y = initialY;

  mut move(x: number, y: number) {
    $x = x;
    $y = y;
  }
}

view DragSubscriber from DragManager {
  <div>
    <span>x: {$x}</span>
    <span>y: {$y}</span>
  </div>
}

view DragContainer {
  <div>
    <DragManager initialX={30} initialY={50}>
      <DragSubscriber />
    </DragManager>
  </div>
}
```

### Object types can also be used to describe dependecies

This allows a component to be used in more generic scenarios.

```
type MovementManager {
  x: number;
  y: number;
}

view MovementSubscriber from MovementManager {
  <div>
    <span>x: {$x}</span>
    <span>y: {$y}</span>
  </div>
}

view DragContainer {
  <div>
    <DragManager initialX={30} initialY={50}>
      <MovementSubscriber />
    </DragManager>
    <SlideManager initialX={40} initialY={30}>
      <MovementSubscriber />
    </SlideManager>
  </div>
}
```
