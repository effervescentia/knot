open Kore;

let __min_port = 1024;
let __max_port = 49151;

let __targets = [
  Target.javascript_es6,
  Target.javascript_common,
  Target.knot,
];

let _check_exists = (name, x) =>
  if (!Sys.file_exists(x)) {
    Fmt.str("%s does not exist: %s", name, x) |> panic;
  };

let _resolve =
    (
      cfg: option(Config.t),
      select: Config.t => 'a,
      default: 'a,
      value: option('a),
    )
    : 'a =>
  switch (cfg, value) {
  | (_, Some(value)) => value
  | (Some(cfg), None) => select(cfg)
  | (None, None) => default
  };

let config_file = (~default=None, ()) => {
  let value = ref(default);
  let argument =
    Argument.create(
      ~alias="c",
      ~default=?default |> Option.map(x => Argument.Value.String(x)),
      config_key,
      String(
        Filename.resolve
        % (
          path => {
            path |> _check_exists("config file");

            value := Some(path);
          }
        ),
      ),
      "the location of a knot config file",
    );
  let resolve = () => value^;

  (argument, resolve);
};

let debug = (~default=ConfigFile.defaults.debug, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.debug)),
      debug_key,
      Unit(() => value := Some(true)),
      "enable a higher level of logging",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.debug, default);

  (argument, resolve);
};

let root_dir = (~default=ConfigFile.defaults.root_dir, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="r",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.root_dir)),
      root_dir_key,
      String(x => value := Some(x)),
      "the root directory to reference modules from",
    );
  let resolve = (cfg: option(Config.t), working_dir: string) => {
    let root_dir =
      value^
      |> _resolve(cfg, x => x.root_dir, default)
      |> Filename.resolve(~cwd=working_dir);

    root_dir |> _check_exists("root directory");

    root_dir;
  };

  (argument, resolve);
};

let port = (~default=ConfigFile.defaults.port, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="p",
      ~default=Int(default),
      ~from_config=cfg => Some(Int(cfg.port)),
      port_key,
      Int(x => value := Some(x)),
      "the port the server runs on",
    );
  let resolve = (cfg: option(Config.t)) => {
    let port = value^ |> _resolve(cfg, x => x.port, default);

    if (port < __min_port || port > __max_port) {
      Fmt.str("port must be in the range of %d to %d", __min_port, __max_port)
      |> panic;
    };

    port;
  };

  (argument, resolve);
};

let source_dir = (~default=ConfigFile.defaults.source_dir, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="s",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.source_dir)),
      source_dir_key,
      String(x => value := Some(x)),
      Fmt.str(
        "the directory to reference source modules from, relative to %a",
        Fmt.bold_str,
        "root-dir",
      ),
    );
  let resolve = (cfg: option(Config.t), root_dir: string) => {
    let source_dir =
      value^
      |> _resolve(cfg, x => x.source_dir, default)
      |> Filename.resolve(~cwd=root_dir);

    source_dir |> _check_exists("source directory");

    if (!String.starts_with(root_dir, source_dir)) {
      Fmt.str(
        "source directory %a must be within root directory %a",
        Fmt.bold_str,
        source_dir,
        Fmt.bold_str,
        root_dir,
      )
      |> panic;
    };

    source_dir;
  };

  (argument, resolve);
};

let _check_entry_exists = _check_exists(entry_key);
let entry = (~default=ConfigFile.defaults.entry, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="e",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.entry)),
      entry_key,
      String(x => value := Some(x)),
      Fmt.str(
        "the entry point for execution, relative to %a",
        Fmt.bold_str,
        "source-dir",
      ),
    );
  let resolve = (cfg: option(Config.t), source_dir) => {
    Namespace.Internal(
      (
        switch (cfg, value^) {
        | (_, Some(value)) =>
          let entry = Filename.resolve(value);
          _check_entry_exists(entry);
          entry |> Filename.relative_to(source_dir);
        | (Some(cfg), None) =>
          let entry = cfg.entry;
          _check_entry_exists(Filename.concat(source_dir, entry));
          entry;
        | (None, None) =>
          let entry = default;
          _check_entry_exists(Filename.concat(source_dir, entry));
          entry;
        }
      )
      |> String.drop_suffix(Constants.file_extension),
    );
  };

  (argument, resolve);
};

let target = () => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="t",
      ~options=__targets,
      ~from_config=
        cfg => cfg.target |?> ~@Target.pp % (x => Argument.Value.String(x)),
      target_key,
      Symbol(__targets, x => value := Some(target_of_string(x))),
      "the target to compile to",
    );
  let resolve = (cfg: option(Config.t)) =>
    switch (cfg, value^) {
    | (_, Some(value)) => value
    | (Some({target: Some(target)}), None) => target
    | (_, _) => panic("must provide a target for compilation")
    };

  (argument, resolve);
};

let out_dir = (~default=ConfigFile.defaults.out_dir, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="o",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.out_dir)),
      out_dir_key,
      String(x => value := Some(x)),
      "the directory to write compiled files to",
    );
  /* no need to check if it exists since we can create it */
  let resolve = (cfg: option(Config.t), root_dir: string) =>
    value^
    |> _resolve(cfg, x => x.out_dir, default)
    |> Filename.resolve(~cwd=root_dir);

  (argument, resolve);
};

let fix = (~default=ConfigFile.defaults.fix, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fix)),
      fix_key,
      Unit(() => value := Some(true)),
      "automatically apply fixes",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.fix, default);

  (argument, resolve);
};

let color = (~default=ConfigFile.defaults.color, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.color)),
      color_key,
      Bool(x => value := Some(x)),
      "allow color in logs",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.color, default);

  (argument, resolve);
};

let fail_fast = (~default=ConfigFile.defaults.fail_fast, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fail_fast)),
      fail_fast_key,
      Unit(() => value := Some(true)),
      "fail as soon as the first error is encountered",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.fail_fast, default);

  (argument, resolve);
};

let log_imports = (~default=ConfigFile.defaults.log_imports, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.log_imports)),
      log_imports_key,
      Unit(() => value := Some(true)),
      "print a graph describing the dependencies between modules",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.log_imports, default);

  (argument, resolve);
};
