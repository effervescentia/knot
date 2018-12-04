open Knot;

let load_resource = file =>
  FileStream.load(Config.resource_dir ++ "/" ++ file);