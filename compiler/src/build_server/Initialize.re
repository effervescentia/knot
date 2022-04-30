open Kore;

type params_t = {root_dir: string};

let deserialize_params =
  JSON.Util.(
    json => {
      let root_dir = json |> member("root_dir") |> to_string;

      {root_dir: root_dir};
    }
  );

let response = () => `Assoc([]);

let handler: Runtime.request_handler_t(params_t) =
  ({find_config, compilers}, {root_dir}) => response() |> Result.ok;
