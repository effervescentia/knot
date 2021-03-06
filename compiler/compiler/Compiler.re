open Core;
open KnotAnalyze.Scope;

module Analyzer = KnotAnalyze.Analyzer;
module Scope = KnotAnalyze.Scope;
module Parser = KnotParse.Parser;

type t = {
  add: string => unit,
  inject: (string, string) => unit,
  status: unit => status,
  complete: unit => unit,
  iter: (string, string, (string, (string, ast_module)) => unit) => unit,
  iter_modules: (string => unit) => unit,
  find: string => option(module_),
  invalidate: string => unit,
  reset: unit => unit,
  debug_modules: unit => unit,
};

let is_loaded = (tbl, key) =>
  Hashtbl.mem(tbl, key)
  && (
    switch (Hashtbl.find(tbl, key)) {
    | Loaded(_) => true
    | _ => false
    }
  );
let is_injected = (tbl, key) =>
  Hashtbl.mem(tbl, key)
  && (
    switch (Hashtbl.find(tbl, key)) {
    | Injected(_) => true
    | _ => false
    }
  );
let is_resolving = (tbl, key) =>
  Hashtbl.mem(tbl, key)
  && (
    switch (Hashtbl.find(tbl, key)) {
    | Resolving => true
    | _ => false
    }
  );
let map_imports = map_name =>
  fun
  | Module(imports, stmts) => {
      let mapped_imports =
        List.map(
          fun
          | Import(module_, targets) => {
              let alias = map_name(module_);
              Import(alias, targets);
            },
          imports,
        );

      Module(mapped_imports, stmts);
    };

let add =
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path} as desc,
      create_desc,
      add_more,
    ) => {
  Log.info("%s  %s (%s)", Emoji.link, pretty_path, relative_path);

  if (is_resolving(global_scope.module_tbl, absolute_path)) {
    throw(CircularDependencyDetected);
  };

  let loaded = Loader.load(Parser.prog, absolute_path);

  Log.info(
    "%s  %s (%s)",
    Emoji.left_pointing_magnifying_glass,
    pretty_path,
    relative_path,
  );

  Hashtbl.add(global_scope.module_tbl, absolute_path, Resolving);
  let module_alias_tbl =
    (
      switch (loaded) {
      | Module(x, _) => List.length(x)
      }
    )
    |> Hashtbl.create;

  let mapped_loaded =
    map_imports(
      name =>
        if (is_injected(global_scope.module_tbl, name)) {
          Hashtbl.add(module_alias_tbl, name, name);
          name;
        } else {
          add_more(name);
          create_desc(name)
          |> (
            ({absolute_path}) => {
              Hashtbl.add(module_alias_tbl, absolute_path, name);

              absolute_path;
            }
          );
        },
      loaded,
    );

  let linked = Linker.link(global_scope, desc, Some(mapped_loaded));

  let mapped_linked = (
    map_imports(name => Hashtbl.find(module_alias_tbl, name), fst(linked)),
    snd(linked),
  );

  Log.info("%s  %s (%s)", Emoji.heavy_check_mark, pretty_path, relative_path);

  Hashtbl.replace(
    global_scope.module_tbl,
    absolute_path,
    Loaded(mapped_linked),
  );
};

let inject =
    (global_scope, {target, absolute_path, relative_path, pretty_path}, name) => {
  Log.info("%s  (%s)", Emoji.syringe, name);

  let loaded = Loader.load(Parser.defn, absolute_path);

  Log.info("%s  (%s)", Emoji.left_pointing_magnifying_glass, name);

  Analyzer.analyze_defn(loaded)
  |> (
    type_ => {
      Log.info("%s  (%s)", Emoji.heavy_check_mark, name);

      Hashtbl.add(global_scope.module_tbl, name, Injected(type_));
    }
  );
};

let create_scope = () =>
  Scope.create(~label="global", ~module_tbl=Hashtbl.create(24), ());

let create = create_desc => {
  let status = ref(Idle);
  let global_scope = ref(create_scope());
  let add_error = (file, pretty_path) =>
    fun
    | CompilationError(e) => {
        let err = (e, file, pretty_path);

        switch (status^) {
        | Failing(xs) => status := Failing([err, ...xs])
        | _ => status := Failing([err])
        };
      }
    | err => raise(err);

  let rec compiler = {
    inject: (path, name) => {
      let desc = create_desc(path);
      let absolute_path = desc.absolute_path;

      if (Filename.extension(absolute_path) == Knot.Constants.typedef_file_ext) {
        try (inject(global_scope^, desc, name)) {
        | err =>
          Hashtbl.replace(global_scope^.module_tbl, name, Failed);

          add_error(path, desc.pretty_path, err);
        };
      };
    },
    add: path => {
      if (status^ != Running) {
        status := Running;
      };

      if (!is_injected(global_scope^.module_tbl, path)) {
        let desc = create_desc(path);
        let {absolute_path} = desc;

        if (!is_loaded(global_scope^.module_tbl, absolute_path)) {
          try (add(global_scope^, desc, create_desc, compiler.add)) {
          | err =>
            Hashtbl.replace(global_scope^.module_tbl, absolute_path, Failed);

            add_error(path, desc.pretty_path, err);
          };
        };
      };
    },
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
        | Loaded((ast, _)) => Some(ast)
        | Failed => throw(InvalidImportTarget(path))
        | _ => None
        };
      } else {
        Hashtbl.to_seq_keys(global_scope^.module_tbl)
        |> Seq.iter(Log.info("member %s"));

        None;
      },
    invalidate: module_ =>
      if (Hashtbl.mem(global_scope^.module_tbl, module_)) {
        if (status^ != Running) {
          status := Running;
        };

        Hashtbl.remove(global_scope^.module_tbl, module_);
      },
    reset: () =>
      if (status^ != Idle) {
        status := Idle;

        global_scope := create_scope();
      },
    debug_modules: () =>
      Hashtbl.iter(
        key =>
          (
            fun
            | Loaded(_) => Printf.sprintf("Loaded(%s)", key)
            | Resolving => Printf.sprintf("Resolving(%s)", key)
            | Failed => Printf.sprintf("Failed(%s)", key)
            | Injected(_) => Printf.sprintf("Injected(%s)", key)
          )
          % Log.info("MODULE: %s"),
        global_scope^.module_tbl,
      ),
  };

  compiler;
};
