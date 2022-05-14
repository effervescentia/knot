open Kore;

let log_config =
    (
      global: Config.global_t,
      name: string,
      attributes: list((string, string)),
    ) => {
  Log.info("running %a command", ~$Fmt.bold(Fmt.green(Fmt.string)), name);
  Log.debug(
    "%s config: %a",
    name,
    ~$Hashtbl.pp(Fmt.string, Fmt.string),
    [
      (color_key, string_of_bool(global.color)),
      (debug_key, string_of_bool(global.debug)),
      ...attributes,
    ]
    |> List.to_seq
    |> Hashtbl.of_seq,
  );
};

let assert_exists = (name, x) =>
  if (!Sys.file_exists(x)) {
    Fmt.str("%s does not exist: %s", name, x) |> panic;
  };
