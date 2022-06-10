open Kore;

type params_t = Protocol.module_params_t;

let method_key = "module/add";

let deserialize = Deserialize.module_params;

let handler: Runtime.notification_handler_t(params_t) =
  (runtime, {path}) =>
    runtime
    |> Util.process_incremental(path, namespace =>
         runtime.compiler
         |> Compiler.upsert_module(namespace)
         |> Tuple.with_fst2([])
       );
