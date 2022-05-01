# `compiler/status`

## Parameters

> *No Parameters*

## Result

```ts
type Status = 'idle' | 'running' | 'complete';
```

- `status` (*Status*): the status of the compiler

## Examples

### Request Example

```json
{
  "jsonrpc": "2.0",
  "id": 99,
  "method": "compiler/status"
}
```

### Response Example

```json
{
  "jsonrpc": "2.0",
  "id": 99,
  "result": {
    "status": "running"
  }
}
```
