open Kore;

type params_t = Protocol.module_params_t;

let method_key = "module/fetch";

let deserialize = Deserialize.module_params;

let response = (data: string) => `Assoc([("data", `String(data))]);

let handler: Runtime.request_handler_t(params_t) =
  (_, _) => {
    response("") |> Result.ok;
  };
