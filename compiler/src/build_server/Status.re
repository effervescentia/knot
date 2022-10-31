type params_t = unit;

let method_key = "compiler/status";

let deserialize = ignore;

let response =
  Runtime.(
    (status: Status.t) =>
      `Assoc([("status", `String(Status.to_string(status)))])
  );

let handler: Runtime.request_handler_t(params_t) =
  ({status, _}, _) => response(status) |> Result.ok;
