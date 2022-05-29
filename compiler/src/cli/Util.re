open Knot.Kore;

let __resource_dirs = Injected.Sites.res;

let find_resource = (~argv=Sys.argv, file: string) =>
  switch (__resource_dirs) {
  | [] => [Sys.argv[0], "../../share/knot/res", file] |> Filename.join
  | [loc, ..._] => Filename.concat(loc, file)
  };
