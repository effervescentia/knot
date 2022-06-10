open Kore;

let __asset_dirs = Injected.Sites.assets;

let find = (~argv=Sys.argv, file: string) =>
  switch (__asset_dirs) {
  | [] => [argv[0], "../../share/knot/assets", file] |> Filename.join
  | [loc, ..._] => Filename.concat(loc, file)
  };
