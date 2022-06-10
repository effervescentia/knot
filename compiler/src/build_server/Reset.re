open Kore;

type params_t = unit;

let method_key = "compiler/reset";

let deserialize = ignore;

let response = () => `Null;

let handler: Runtime.request_handler_t(params_t) =
  (runtime, _) => {
    Compiler.reset(runtime.compiler);
    Compiler.add_standard_library(runtime.compiler);
    runtime.status = Idle;

    response() |> Result.ok;
  };
