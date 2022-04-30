open Kore;

type t =
  | Initialize(int, Initialize.params_t)
  | Hover(int, Hover.params_t)
  | GoToDefinition(int, GoToDefinition.params_t)
  | CodeCompletion(int, CodeCompletion.params_t)
  | LocalSymbols(int, LocalSymbols.params_t)
  | WorkspaceSymbols(int, WorkspaceSymbols.params_t)
  | Format(int, Format.params_t)
  | FileOpen(FileOpen.params_t)
  | FileClose(FileClose.params_t)
  | FileChange(FileChange.params_t);

let of_initialize = (id, params) => Initialize(id, params);
let of_hover = (id, params) => Hover(id, params);
let of_go_to_definition = (id, params) => GoToDefinition(id, params);
let of_code_completion = (id, params) => CodeCompletion(id, params);
let of_local_symbols = (id, params) => LocalSymbols(id, params);
let of_workspace_symbols = (id, params) => WorkspaceSymbols(id, params);
let of_format = (id, params) => Format(id, params);
let of_file_open = params => FileOpen(params);
let of_file_close = params => FileClose(params);
let of_file_change = params => FileChange(params);

let deserialize =
  JSONRPC.Protocol.Event.(
    fun
    | Request(id, method, params) when method == Initialize.method_key =>
      params |> Initialize.deserialize |> of_initialize(id) |> Option.some

    | Request(id, method, params) when method == Hover.method_key =>
      params |> Hover.deserialize |> of_hover(id) |> Option.some

    | Request(id, method, params) when method == GoToDefinition.method_key =>
      params
      |> GoToDefinition.deserialize
      |> of_go_to_definition(id)
      |> Option.some

    | Request(id, method, params) when method == CodeCompletion.method_key =>
      params
      |> CodeCompletion.deserialize
      |> of_code_completion(id)
      |> Option.some

    | Request(id, method, params) when method == LocalSymbols.method_key =>
      params
      |> LocalSymbols.deserialize
      |> of_local_symbols(id)
      |> Option.some

    | Request(id, method, params) when method == WorkspaceSymbols.method_key =>
      params
      |> WorkspaceSymbols.deserialize
      |> of_workspace_symbols(id)
      |> Option.some

    | Request(id, method, params) when method == Format.method_key =>
      params |> Format.deserialize |> of_format(id) |> Option.some

    | Request(id, method, params) when method == FileOpen.method_key =>
      params |> FileOpen.deserialize |> of_file_open |> Option.some

    | Request(id, method, params) when method == FileClose.method_key =>
      params |> FileClose.deserialize |> of_file_close |> Option.some

    | Request(id, method, params) when method == FileChange.method_key =>
      params |> FileChange.deserialize |> of_file_change |> Option.some

    | _ => None
  );
