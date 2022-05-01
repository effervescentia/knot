open Kore;
open JSON.Util;

let module_params = (json): Protocol.module_params_t => {
  let path = json |> member("path") |> to_string;

  {path: path};
};
