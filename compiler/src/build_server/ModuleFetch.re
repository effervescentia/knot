open Kore;

module Generator = Generate.Generator;

type params_t = Protocol.module_params_t;

let method_key = "module/fetch";

let deserialize = Deserialize.module_params;

let response = (data: string) => `Assoc([("data", `String(data))]);

let handler: Runtime.request_handler_t(params_t) =
  ({compiler}, {path}) => {
    let namespace = Namespace.of_string(path);

    switch (compiler |> Compiler.get_module(namespace)) {
    | Some({ast}) =>
      ast
      |> ~@
           Generator.pp(
             Target.Knot,
             fun
             | Internal(path) => path
             | External(_) => raise(NotImplemented),
           )
      |> response
      |> Result.ok
    | None => Error(id => JSONRPC.Protocol.builtin_error(~id, InternalError))
    };
  };
