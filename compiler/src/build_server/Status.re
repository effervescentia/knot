open Kore;

type params_t = unit;

let method_key = "compiler/status";

let deserialize = ignore;

let response = () => `Null;

let handler: Runtime.request_handler_t(params_t) =
  (_, _) => response() |> Result.ok;
