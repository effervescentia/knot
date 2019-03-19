open Core;
open KnotAnalyze.Scope;

module Scope = KnotAnalyze.Scope;
module Parser = KnotParse.Parser;

type status =
  | Idle
  | Running
  | Complete;

type t = {
  add: string => list(string),
  inject: (string, string) => unit,
  add_rec: string => unit,
  status: unit => status,
  complete: unit => unit,
  iter: (string, string, (string, (string, ast_module)) => unit) => unit,
  iter_modules: (string => unit) => unit,
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

  let loaded = Loader.load(Parser.prog, absolute_path);

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
          absolute_path,
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

let inject =
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path} as desc,
      name,
    ) => {
  Log.info("%s  %s (%s)", Emoji.syringe, pretty_path, name);

  let loaded = Loader.load(Parser.defn, absolute_path);

  Log.info(
    "%s  %s (%s)",
    Emoji.left_pointing_magnifying_glass,
    pretty_path,
    name,
  );

  Linker.link_defn(global_scope, desc, loaded)
  |> (
    type_ => {
      Log.info("%s  %s (%s)", Emoji.heavy_check_mark, pretty_path, name);

      Hashtbl.add(global_scope.module_tbl, name, Injected(type_));
    }
  );
};

let create_scope = () =>
  Scope.create(~label="global", ~module_tbl=Hashtbl.create(24), ());

let create = create_desc => {
  let status = ref(Idle);
  let global_scope = ref(create_scope());

  let rec compiler = {
    inject: (path, name) => {
      let desc = create_desc(path);
      let absolute_path = desc.absolute_path;

      if (Filename.extension(absolute_path) == ".kd") {
        inject(global_scope^, desc, name);
      };
    },
    add: path => {
      if (status^ != Running) {
        status := Running;
      };

      let desc = create_desc(path);
      let absolute_path = desc.absolute_path;

      if (Hashtbl.mem(global_scope^.module_tbl, absolute_path)) {
        [];
      } else {
        add(global_scope^, desc);
      };
    },
    add_rec: path => compiler.add(path) |> List.iter(compiler.add_rec),
    status: () => status^,
    complete: () => status := Complete,
    iter: (entry, source_dir, f) =>
      BuildTbl.extract(entry, source_dir, global_scope^.module_tbl)
      |> Hashtbl.iter(f),
    iter_modules: f =>
      Hashtbl.iter((key, _) => f(key), global_scope^.module_tbl),
    find: path =>
      if (Hashtbl.mem(global_scope^.module_tbl, path)) {
        switch (Hashtbl.find(global_scope^.module_tbl, path)) {
        | Loaded(_, (ast, _)) => Some(ast)
        | _ => None
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
