open Kore;

module Status = {
  type t =
    | None
    | Pending
    | Purged
    | Valid
    | Partial
    | Invalid;

  let to_string =
    fun
    | None => "none"
    | Pending => "pending"
    | Purged => "purged"
    | Valid => "valid"
    | Partial => "partial"
    | Invalid => "invalid";
};

type params_t = Protocol.module_params_t;

let method_key = "module/status";

let deserialize = Deserialize.module_params;

let response = (status: Status.t) =>
  `Assoc([("status", `String(Status.to_string(status)))]);

let handler: Runtime.request_handler_t(params_t) =
  ({compiler} as runtime, {path}) => {
    let namespace = runtime |> Util.resolve_namespace(path) |> Result.get_ok;

    (
      switch (compiler |> Compiler.get_module(namespace)) {
      | Some(Valid(_)) => Status.Valid
      | Some(Partial(_)) => Status.Partial
      | Some(Invalid(_)) => Status.Invalid
      | Some(Pending) => Status.Pending
      | Some(Purged) => Status.Purged
      | Some(Library(_)) => Status.None
      | None =>
        compiler.graph |> Resolve.ImportGraph.has_module(namespace)
          ? Status.Pending : Status.None
      }
    )
    |> response
    |> Result.ok;
  };
