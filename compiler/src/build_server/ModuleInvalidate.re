open Kore;

type params_t = Protocol.module_params_t;

let method_key = "module/invalidate";

let deserialize = Deserialize.module_params;

let handler: Runtime.notification_handler_t(params_t) =
  ({compiler}, {path}) => {
    let namespace = Namespace.of_string(path);

    compiler |> Compiler.remove_module(namespace) |> ignore;
    compiler |> Compiler.add_module(namespace) |> ignore;
  };
