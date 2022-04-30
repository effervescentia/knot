open Kore;

type t =
  | Initialize(int, Initialize.params_t);

let of_init = (id, params) => Initialize(id, params);

let deserialize =
  JSONRPC.Protocol.Event.(
    fun
    | Request(id, key, params) when key == Initialize.method_key =>
      params |> Initialize.deserialize |> of_init(id) |> Option.some
    | _ => None
  );
