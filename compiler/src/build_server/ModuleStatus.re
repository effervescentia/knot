open Kore;

type params_t = Protocol.module_params_t;

let method_key = "module/status";

let deserialize = Deserialize.module_params;

let response = (status: string) => `Assoc([("status", `String(status))]);

let handler: Runtime.request_handler_t(params_t) =
  (_, _) => response("") |> Result.ok;
