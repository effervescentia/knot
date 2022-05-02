open Kore;

type params_t = Protocol.module_params_t;

let method_key = "module/add";

let deserialize = Deserialize.module_params;

let handler: Runtime.notification_handler_t(params_t) =
  ({compiler}, {path}) =>
    compiler |> Compiler.upsert_module(Namespace.of_string(path)) |> ignore;
