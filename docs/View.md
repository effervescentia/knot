# View

An entity that accepts a keyed `$Type` and returns an `$Element`.

## Type Signature

Property order doesn't matter, it always produces the same type signature.
Properties can be spread in order to provide a concrete `type` for them.

```
view SimpleView(age: number, name: string) {
  <div />
}

// is equivalent to

view AlternateView(name: string, age: number) {
  <div />
}

// and

type SpreadType {
  age: number;
  name: string;
}

view SpreadView(...props: SpreadType) {
  <div />
}

SimpleView.Props;     // $Type(age: number, name: string)
AlternateView.Props;  // $Type(age: number, name: string)
SpreadView.Props;     // SpreadType -> $Type(age: number, name: string)

SimpleView;
// $View<SimpleView.Props, SimpleView()>
//   -> $View<$Type(age: number, name: string), SimpleView()>

AlternateView;
// $View<AlternateView.Props, AlternateView()>
//   -> $View<$Type(age: number, name: string), AlternateView()>

SpreadView;
// $View<SpreadView.Props, SpreadView()>
//   -> $View<SpreadType, SpreadView()>
//   -> $View<$Type(age: number, name: string), SpreadView()>
```

## Keys

In order to differentiate elements for more efficient re-rendering, keys
can be provided (this might not actually be necessary based on how we build the
dependency graphs).

```
view ListView(items: string[]) {
  <ul class="list">
    {items.map((item, index) => {
      <li({index}) class="list__item">
        <span>{item}</span>
      </li>
    }}
  </ul>
}
```

## Omit Parentheses

When your view does not accept properties, the parentheses can be omitted.

```
view SimpleView() { /* ... */ } // not preferred

view SimpleView { /* ... */ }   // preferred
```

## Explicit or Implicit JSX Return

JSX is automatically returned if it is the only expression and doesn't require
an explicit `return` statement.

```
view SimpleView {
  <div>123</div>
}

// is equivalent to

view ExplicitReturnView {
  let x = '123';

  return <div>{x}</div>;
}
```

## Rendering Nothing

To have your `view` not render anything, either explicitly or implicitly return nothing.

```
view ExplicitEmptyView {
  // do something

  return;
}

// or

view ImplicitEmptyView {
  // do something

  // implicit empty return
}
```

## View Composition

Building your `view`'s with composition in mind allows you to re-use them easily and
combine them with other `view`'s to create complex experiences for your users.

### Slots

Composition is achieved by using special `<slot>` tags to inject content placed
within the containing `view`. This allows `view`'s to be more easily re-used, such as
a complex "button" that can have different textual content injected into it.

```
view WrappingView {
  <div class="wrapper">
    <slot />
  </div>
}

view ComposingView {
  <h1>
    <WrappingView>
      <span>Inner Content</span>
    </WrappingView>
  </h1>
}

// ComposingView() ->
//  <h1>
//    <div class="wrapper">
//      <span>Inner Content</span>
//    </div>
//  </h1>
```

### Named Slots

In order to inject multiple different pieces of content, slots can be named and
matched to the keys provided on elements placed into the `view`.

```
view ModalView {
  <div class="modal">
    <h1 class="modal__header">
      <slot(header) />
    </h1>
    <div class="modal__body">
      <slot(body) />
    <div>
    <div class="modal__footer">
      <slot(footer) />
    <div>
  </div>
}

view AlertModal {
  <Modal>
    <span(header)>This is a modal</span>
    <div(body)>
      <p>some content...</p>
      <p>some more content...</p>
    </div>
    <div(footer)>
      <button>Cancel</button>
      <button>OK</button>
    </div>
  </Modal>
}

// AlertModal() ->
//  <div class="modal">
//    <h1 class="modal__header">
//      <span>This is a modal</span>
//    </h1>
//    <div class="modal__body">
//      <div>
//        <p>some content...</p>
//        <p>some more content...</p>
//      </div>
//    <div>
//    <div class="modal__footer">
//      <div>
//        <button>Cancel</button>
//        <button>OK</button>
//      </div>
//    <div>
//  </div>
```

### Default Content

Slots can be provided with default content that is replaced if overridden
by the containing `view`.

```
view NamedSlotView {
  <div class="named-slot">
    <slot(optional)>
      <span>some default content</span>
    </slot>
  <div>
}

view NonComposingView {
  <NamedSlotView />
}

view ComposingView {
  <NamedSlotView>
    <h1>overridden content</h1>
  </NamedSlotView>
}

// NonComposingView() ->
//  <div class="named-slot">
//    <span>some default content</span>
//  </div>

// ComposingView() ->
//  <div class="named-slot">
//    <h1>overridden content</h1>
//  </div>
```

## Alternate Composition

The above solution might not be so good because it ties you more closely to only being able to
use the `children` in a very rigid way. This alternative pattern sticks closer to
`React`'s `children`.

### Slots

The keyword `children` is available within any `view` to easily access the
children placed into it.

```
view WrappingView {
  <div class="wrapper">
    {children}
  </div>
}

view ComposingView {
  <h1>
    <WrappingView>
      <span>Inner Content</span>
    </WrappingView>
  </h1>
}

// ComposingView() ->
//  <h1>
//    <div class="wrapper">
//      <span>Inner Content</span>
//    </div>
//  </h1>
```

### Named Slots

In order to inject multiple different pieces of content, `children` can be selected
based on their keys.

```
view ModalView {
  <div class="modal">
    <h1 class="modal__header">
      {children.header}
    </h1>
    <div class="modal__body">
      {children.body}
    <div>
    <div class="modal__footer">
      {children.footer}
    <div>
  </div>
}

view AlertModal {
  <Modal>
    <span(header)>This is a modal</span>
    <div(body)>
      <p>some content...</p>
      <p>some more content...</p>
    </div>
    <div(footer)>
      <button>Cancel</button>
      <button>OK</button>
    </div>
  </Modal>
}

// AlertModal() ->
//  <div class="modal">
//    <h1 class="modal__header">
//      <span>This is a modal</span>
//    </h1>
//    <div class="modal__body">
//      <div>
//        <p>some content...</p>
//        <p>some more content...</p>
//      </div>
//    <div>
//    <div class="modal__footer">
//      <div>
//        <button>Cancel</button>
//        <button>OK</button>
//      </div>
//    <div>
//  </div>
```

## Extend or Implement Other Views

Override or extend the signature and functionality of another `view`.

```
view BaseView(x: string, y: string) {
  <div>You are the {x} to my {y}</div>
}
```

### Property Type Inheritance

Properties by the same name as the super `view` inherit the same types by default.

```
view CopyView(x, y) : BaseView {
  x; // String
  y; // String

  // implicit "empty" return
}
```

### Simple Property Transformation

Easily transform properties before applying them to a base view.

```
view SimpleExtendingView(x, y) : BaseView(x: `x: {x}`, y);
```

### Partially Apply Properties

Properties can be applied during `view` declaration. If no properties are passed
to the extended `view`, the parentheses can be omitted.

```
view PartialApplicationExtendingView(x, y) : BaseView(x) {
  super(y: `my value {y}`)
}

// or without partial application

view NoPartialApplicationExtendingView(x, y) : BaseView {
  let formatted = `my value {y}`;

  return super(x, y: formatted);
}
```

### Extend and Augment Signature

Override and the property signature of the extended `view`.

```
view AugmentedExtendingView(x, y: number, z: string) : BaseView {
  let formatted = `my value {y} with {z}`;

  return super(x, y: formatted);
}

AugmentedExtendingView.Props; // $Type(x: string, y: number, z: string)
```

### Extension as an Interface

Use extend another `view` for its signature.

```
view InterfacingView(x, y) : BaseView {
  <span>Some other {x} of {y}</span>
}
```

### Nest Super Content

Nest content from super `view` inside of more tags.

```
view NestedExtendingView(x, y) : BaseView(x, y) {
  <h1 id="nested">
    {super()}
  </h1>
}
```

## Full view declaration syntax

> NOTE: consider an alternative to `with` such as the `&` character.

<!-- mix `from` and `with` to just have one subscription format -->

```
view ComplexView(a, b, c) : BaseView(a, b) with SomeState, OtherState(), SomeProvider, OtherProvider(c) {

}
```

## Create State explicitly (bypass dependency injection)

Ignore grabbing a state from the currect scope by partially or fully constructing one in the
declaration of the view.

```
state MyState(y: string, x: number) { }

// force a local state while receiving parameters from a provider / multiple partial application providers
view SomeView with MyState() { }

view PartialConstructionView with MyState#(x: 2) { }
view PartialConstructionView with MyState('abc') { }
```

## Partial Context Application

```
state ValueProvider($x: number, $y: number) { }

view TopLevel {
  <ValueProvider x={123}>
    <div>
      <ChildComponent />
    </div>
  </ValueProvider>
}

view ChildComponent {
  <ValueProvider y={657}>
    <span>
      <DependentComponent />
    </span>
  </ValueProvider>
}

view DependentComponent with ValueProvider {
  <div>
    <span>x: {$x}</span>
    <span>y: {$y}</span>
  </div>
}
```
