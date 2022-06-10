# `module/status`

## Parameters

- `path` (*string*): absolute path to the module

## Result

```ts
type ModuleStatus = 'none' | 'pending' | 'purged' | 'valid' | 'partial' | 'invalid';
```

- `status` (*ModuleStatus*): the compilation status of the module

## Examples

### Request Example

```json
{
  "jsonrpc": "2.0",
  "id": 99,
  "method": "module/status",
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
    "status": "valid"
  }
}
```
