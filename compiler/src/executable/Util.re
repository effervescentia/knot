open Kore;

let extract_global_config = (global: Config.global_t) => [
  (name_key, global.name),
  (color_key, string_of_bool(global.color)),
  (debug_key, string_of_bool(global.debug)),
];

let pp_attributes = ppf => Fmt.struct_(Fmt.string, Fmt.string, ppf);

let log_config =
    (
      global: Config.global_t,
      name: string,
      attributes: list((string, string)),
    ) => {
  Log.info("running command %s", name |> ~@Fmt.info_str);
  Log.debug(
    "command configuration\n%s",
    (
      Fmt.str("%sConfig", String.to_pascal_case(name)),
      extract_global_config(global) @ attributes,
    )
    |> ~@pp_attributes,
  );
};

let assert_exists = (arg_key, x) =>
  if (!Sys.file_exists(x)) {
    InvalidArgument(
      arg_key,
      Fmt.str("file %a does not exist", Fmt.bold_str, x),
    )
    |> fatal;
  };
