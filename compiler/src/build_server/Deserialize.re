open Kore;

let module_params =
  Yojson.Basic.Util.(
    (json) => (
      {
        let path = json |> member("path") |> to_string;

        {path: path};
      }: Protocol.module_params_t
    )
  );
