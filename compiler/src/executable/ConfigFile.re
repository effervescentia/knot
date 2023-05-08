/**
 Parser for `.knot.yml` config files.
 */
open Kore;

type config_file_error_t =
  | CannotParse
  | UnexpectedProperty(string)
  | InvalidFormat;

exception ConfigFileError(config_file_error_t);

let get_defaults = (~color=!is_ci_env, ()) => {...Config.defaults, color};

let find = File.Util.find_up(config_file_name);

let describe_error =
  fun
  | CannotParse => "failed to parse as valid YAML"

  | UnexpectedProperty(name) =>
    Fmt.str("invalid configuration file property found: %s", name)

  | InvalidFormat =>
    Fmt.str(
      "expected an object with some of the following keys: %s, %s, %s",
      name_key,
      root_dir_key,
      target_key,
    );

let read =
    (~defaults=Config.defaults, file: string)
    : result(Config.t, config_file_error_t) => {
  let project_name = ref(defaults.name);
  let root_dir = ref(defaults.root_dir);
  let source_dir = ref(defaults.source_dir);
  let out_dir = ref(defaults.out_dir);
  let target = ref(defaults.target);
  let entry = ref(defaults.entry);
  let color = ref(defaults.color);
  let fix = ref(defaults.fix);
  let fail_fast = ref(defaults.fail_fast);
  let log_imports = ref(defaults.log_imports);
  let debug = ref(defaults.debug);
  let port = ref(defaults.port);

  let build_config = () =>
    Config.{
      name: project_name^,
      root_dir: root_dir^,
      source_dir: source_dir^,
      out_dir: out_dir^,
      target: target^,
      entry: entry^,
      color: color^,
      fix: fix^,
      fail_fast: fail_fast^,
      log_imports: log_imports^,
      debug: debug^,
      port: port^,
    };

  switch (
    switch (file |> File.IO.read_to_string |> Yaml.of_string) {
    | Ok(`O(entries)) =>
      entries
      |> List.iter(
           fun
           | (name, `String(value)) when name == name_key =>
             project_name := Some(value)
           | (name, `String(value)) when name == root_dir_key =>
             root_dir := value
           | (name, `String(value)) when name == source_dir_key =>
             source_dir := value
           | (name, `String(value)) when name == out_dir_key =>
             out_dir := value
           | (name, `String(value)) when name == entry_key => entry := value
           | (name, `String(value)) when name == target_key =>
             target := Some(target_of_string(value))

           | (name, `Bool(value)) when name == fix_key => fix := value
           | (name, `Bool(value)) when name == fail_fast_key =>
             fail_fast := value
           | (name, `Bool(value)) when name == log_imports_key =>
             log_imports := value
           | (name, `Bool(value)) when name == color_key => color := value
           | (name, `Bool(value)) when name == debug_key => debug := value

           | (name, `Float(value)) when name == port_key =>
             port := value |> int_of_float

           | (name, _) => raise(ConfigFileError(UnexpectedProperty(name))),
         );

      build_config();

    | Ok(`Null) => build_config()

    | Ok(_) => raise(ConfigFileError(InvalidFormat))

    | _ => raise(ConfigFileError(CannotParse))
    }
  ) {
  | value => Ok(value)

  | exception (ConfigFileError(err)) => Error(err)
  };
};
