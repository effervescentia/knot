open Kore;

module Generator = Generate.Generator;

type params_t = Protocol.module_params_t;

let method_key = "module/fetch";

let deserialize = Deserialize.module_params;

let response = (data: string) => `Assoc([("data", `String(data))]);

let handler: Runtime.request_handler_t(params_t) =
  ({compiler, target}, {path}) => {
    let namespace = Namespace.of_path(path);

    compiler
    |> Compiler.get_module(namespace)
    |?< ModuleTable.(get_entry_data % Option.map(({ast}) => ast))
    |?> ~@
          Generator.pp(
            target,
            fun
            | Internal(path) => path
            | External(_) => raise(NotImplemented),
          )
    |?> response
    |> Option.to_result(~none=id =>
         JSONRPC.Protocol.builtin_error(~id, InternalError)
       );
  };
