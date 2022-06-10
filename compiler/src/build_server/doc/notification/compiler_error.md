# `compiler/error`

## Parameters

```ts
interface CompilerError {
  type: string;
  message: string;
}

interface ModuleError {
  type: string;
  path: string;
  message: string;
}

type AnyError = CompilerError | ModuleError;
```

- `errors` (*AnyError[]*): list of errors to report

## Example

```json
{
  "jsonrpc": "2.0",
  "method": "compiler/error",
  "params": {
    "errors": [
      {
        "type": "unknown identifier",
        "path": "/path/to/file.kn",
        "message": "unable to find a variable in scope with identifier"
      },
      {
        "type": "module not found",
        "message": "unable to resolve module @/foo"
      }
    ]
  }
}
```
