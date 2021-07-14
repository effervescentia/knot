/**
 Parse command line args into config.
 */
open Kore;

let name = ".knot.yml";

let find = File.Util.find_up(name);

let read = (file: string): Config.t => {
  let root_dir = ref(file |> Filename.dirname);
  let project_name = ref(default_config.name);
  let source_dir = ref(default_config.source_dir);
  let out_dir = ref(default_config.out_dir);
  let target = ref(default_config.target);
  let entry = ref(default_config.entry);
  let color = ref(default_config.color);
  let fix = ref(default_config.fix);
  let fail_fast = ref(default_config.fail_fast);
  let debug = ref(default_config.debug);
  let port = ref(default_config.port);

  Log.debug("looking for config file: %s", file);

  switch (file |> File.IO.read_to_string |> Yaml.of_string) {
  | Ok(`O(entries)) =>
    entries
    |> List.iter(
         fun
         | (name, `String(value)) when name == name_key =>
           project_name := Some(value)
         | (name, `String(value)) when name == root_dir_key =>
           root_dir := value |> Filename.resolve
         | (name, `String(value)) when name == source_dir_key =>
           source_dir := value
         | (name, `String(value)) when name == out_dir_key =>
           out_dir := value

         | (name, `String(value)) when name == target_key =>
           target := Some(target_of_string(value))
         | (name, `String(value)) when name == entry_key => entry := value
         | (name, `Bool(value)) when name == fix_key => fix := value
         | (name, `Bool(value)) when name == fail_fast_key =>
           fail_fast := value
         | (name, `Bool(value)) when name == color_key => color := value
         | (name, `Bool(value)) when name == debug_key => debug := value
         | (name, `Float(value)) when name == port_key =>
           port := value |> int_of_float
         | (name, _) =>
           name |> Print.fmt("invalid entry found: %s") |> panic,
       )
  | Ok(`Null) => ()
  | Ok(_) =>
    Print.fmt(
      "expected an object with some of the following keys: root_dir, source_dir, entry",
    )
    |> panic
  | _ => file |> Print.fmt("failed to parse configuration file: %s") |> panic
  };

  {
    name: project_name^,
    root_dir: root_dir^,
    source_dir: source_dir^,
    out_dir: out_dir^,
    target: target^,
    entry: entry^,
    color: color^,
    fix: fix^,
    fail_fast: fail_fast^,
    debug: debug^,
    port: port^,
  };
};
