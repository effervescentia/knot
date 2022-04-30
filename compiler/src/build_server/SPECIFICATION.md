# Requests

## `initialize`

### Parameters

- `root_dir` (*string*): the directory containing the project's `.knot.yml` file

### Examples

```json

// Request Example
{
  "id": 99,
  "method": "initialize",
  "params": {
    "root_dir": "/path/to/project"
  }
}

// Response Example
{
  "jsonrpc": "2.0",
  "id": 99,
  "results": {}
}
```
