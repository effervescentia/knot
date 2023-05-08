# `module/fetch`

## Parameters

- `path` (*string*): absolute path to the module

## Result

- `data` (*string*): the compiled module output

## Examples

### Request Example

```json
{
  "jsonrpc": "2.0",
  "id": 99,
  "method": "module/fetch",
  "params": {
    "path": "/path/to/file.kn"
  }
}
```

### Response Example

```json
{
  "jsonrpc": "2.0",
  "id": 99,
  "result": {
    "data": "var foo = 'bar';"
  }
}
```
