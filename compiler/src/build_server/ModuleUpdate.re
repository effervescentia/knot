open Kore;

type params_t = Protocol.module_params_t;

let method_key = "module/update";

let deserialize = Deserialize.module_params;

let handler: Runtime.notification_handler_t(params_t) =
  (runtime, {path}) =>
    runtime
    |> Util.process_incremental(() => {
         let namespace = Namespace.of_path(path);

         runtime.compiler
         |> Compiler.update_module(namespace)
         |> Tuple.with_fst2([]);
       });
