# Styling Views

```
style Main {
  .root {
    backgroundColor: red;
  }
}

view MyView ~ Main -> (
  <div.root>
    <span>{"some text"}</span>
  </div.root>
)
```
