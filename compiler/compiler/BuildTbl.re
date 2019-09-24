open Core;

module Context = KnotResolve.Context;

let _initial_size = 16;

let extract = (entry, source_dir, tbl) =>
  Hashtbl.fold(
    (key, value, acc) => {
      switch (value) {
      /* internal modules */
      | Loaded(ast) when Util.is_source_module(key) =>
        Hashtbl.add(acc, Util.to_path_segment(key), (key, ast))
      /* external modules */
      | Loaded(ast) when key == entry =>
        Hashtbl.add(
          acc,
          Util.chop_path_prefix(source_dir, key) |> Filename.chop_extension,
          (Knot.Constants.main_module_alias, ast),
        )
      /* ignore all other modules */
      | _ => ()
      };

      acc;
    },
    tbl,
    Hashtbl.create(_initial_size),
  );
