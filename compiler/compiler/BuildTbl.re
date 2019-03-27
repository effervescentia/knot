open Core;

module Context = KnotResolve.Context;

let extract = (entry, source_dir, tbl) =>
  Hashtbl.fold(
    (key, value, acc) => {
      switch (value) {
      | Loaded(ast) when Util.is_source_module(key) =>
        Hashtbl.add(acc, Util.to_path_segment(key), (key, ast))
      | Loaded(ast) when key == entry =>
        Hashtbl.add(
          acc,
          Util.chop_path_prefix(source_dir, key) |> Filename.chop_extension,
          (main_alias, ast),
        )
      | _ => ()
      };

      acc;
    },
    tbl,
    Hashtbl.create(16),
  );
