open Knot;

let load_resource = file =>
  FileStream.load(Config.resource_dir ++ "/" ++ file);

let load_resource2 = file => open_in(Config.resource_dir ++ "/" ++ file);