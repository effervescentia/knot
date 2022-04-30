open Kore;

let init_key = "initialize";

type t =
  | Initialize(int, Initialize.params_t);

let of_init = (id, params) => Initialize(id, params);

let deserialize =
  JSONRPC.Protocol.Event.(
    fun
    | Request(id, key, params) when key == init_key =>
      params |> Initialize.deserialize_params |> of_init(id) |> Option.some
    | _ => None
  );
