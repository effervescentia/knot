open Core;
open KnotAnalyze.Scope;

module Scope = KnotAnalyze.Scope;

type t = {
  add: string => list(string),
  is_complete: unit => bool,
  iter: (string, string, (string, (string, ast_module)) => unit) => unit,
  iter_pending: (resolve_target => unit) => unit,
};

let add =
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path} as desc,
    ) =>
  if (!Hashtbl.mem(global_scope.module_tbl, target)) {
    Log.info("%s  %s (%s)", Emoji.link, pretty_path, relative_path);

    let loaded = Loader.load(absolute_path);

    Log.info(
      "%s  %s (%s)",
      Emoji.left_pointing_magnifying_glass,
      pretty_path,
      relative_path,
    );

    Linker.link(global_scope, desc, loaded)
    |> (
      ((deps, ast)) => {
        switch (ast) {
        | Some(x) =>
          Log.info(
            "%s  %s (%s)",
            Emoji.heavy_check_mark,
            pretty_path,
            relative_path,
          );

          Hashtbl.add(
            global_scope.module_tbl,
            target,
            Loaded(absolute_path, x),
          );
        | None =>
          Log.info(
            "%s  %s (%s)",
            Emoji.hourglass_with_flowing_sand,
            pretty_path,
            relative_path,
          )
        };

        deps;
      }
    );
  } else {
    [];
  };

let create = create_desc => {
  let module_tbl = Hashtbl.create(24);
  let global_scope = Scope.create(~label="global", ~module_tbl, ());

  {
    add: create_desc % add(global_scope),
    is_complete: global_scope.is_resolved,
    iter: (entry, source_dir, f) =>
      BuildTbl.extract(entry, source_dir, module_tbl) |> Hashtbl.iter(f),
    iter_pending: f => global_scope.pending() |> List.iter(f),
  };
};
