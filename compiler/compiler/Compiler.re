open Core;
open KnotAnalyze.Scope;

module Scope = KnotAnalyze.Scope;

type status =
  | Idle
  | Running
  | Complete;

type t = {
  add: string => list(string),
  add_rec: string => unit,
  is_complete: unit => bool,
  status: unit => status,
  iter: (string, string, (string, (string, ast_module)) => unit) => unit,
  iter_modules: (string => unit) => unit,
  iter_pending: (resolve_target => unit) => unit,
  find: string => option(module_),
  invalidate: string => unit,
  reset: unit => unit,
};

let add =
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path} as desc,
    ) => {
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
};

let create_scope = () =>
  Scope.create(~label="global", ~module_tbl=Hashtbl.create(24), ());

let create = create_desc => {
  let status = ref(Idle);
  let global_scope = ref(create_scope());

  let rec compiler = {
    add: path => {
      Log.info("AFSLDASKD %s", path);

      if (status^ != Running) {
        status := Running;
      };

      if (Hashtbl.mem(global_scope^.module_tbl, path)) {
        [];
      } else {
        create_desc(path) |> add(global_scope^);
      };
    },
    add_rec: path => {
      compiler.add(path) |> List.iter(compiler.add_rec);

      if (compiler.is_complete()) {
        status := Complete;

        Log.info("%s  compiled!", Emoji.input_numbers);
      };
    },
    status: () => status^,
    is_complete: global_scope^.is_resolved,
    iter: (entry, source_dir, f) =>
      BuildTbl.extract(entry, source_dir, global_scope^.module_tbl)
      |> Hashtbl.iter(f),
    iter_modules: f =>
      Hashtbl.iter((key, _) => f(key), global_scope^.module_tbl),
    iter_pending: f => global_scope^.pending() |> List.iter(f),
    find: path =>
      if (Hashtbl.mem(global_scope^.module_tbl, path)) {
        switch (Hashtbl.find(global_scope^.module_tbl, path)) {
        | Loaded(_, ast) => Some(abandon_ctx(ast))
        | NotLoaded(_) => None
        };
      } else {
        Hashtbl.to_seq_keys(global_scope^.module_tbl)
        |> Seq.iter(Log.info("member %s"));

        None;
      },
    invalidate: module_ => {
      if (Hashtbl.mem(global_scope^.module_tbl, module_)) {
        if (status^ != Running) {
          status := Running;
        };

        Hashtbl.remove(global_scope^.module_tbl, module_);
      };

      compiler.add_rec(module_);
    },
    reset: () =>
      if (status^ != Idle) {
        status := Idle;

        global_scope := create_scope();
      },
  };

  compiler;
};
