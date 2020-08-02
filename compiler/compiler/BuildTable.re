open Globals;

let _initial_size = 16;

let extract = (entry, source_dir, tbl) =>
  Hashtbl.fold(
    (key, value, acc) => {
      switch (value) {
      /* internal modules */
      | Loaded(ast) when FileUtil.is_source_module(key) =>
        Hashtbl.add(acc, FileUtil.to_path_segment(key), (key, ast))
      /* external modules */
      | Loaded(ast) when key == entry =>
        Hashtbl.add(
          acc,
          FileUtil.relative_path(source_dir, key) |> Filename.chop_extension,
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
