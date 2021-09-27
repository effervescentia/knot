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

let debug = (~default=default_config.debug, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.debug)),
      debug_key,
      Unit(() => value := Some(true)),
      "enable a higher level of logging",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.debug, default);

  (opt, resolve);
};

let root_dir = (~default=default_config.root_dir, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~alias="r",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.root_dir)),
      root_dir_key,
      String(x => value := Some(x)),
      "the root directory to reference modules from",
    );
  let resolve = (cfg: option(Config.t)) => {
    let root_dir =
      value^ |> _resolve(cfg, x => x.root_dir, default) |> Filename.resolve;

    if (!Sys.file_exists(root_dir)) {
      root_dir |> Fmt.str("root directory does not exist: %s") |> panic;
    };

    root_dir;
  };

  (opt, resolve);
};

let port = (~default=default_config.port, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
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

  (opt, resolve);
};

let _check_source_dir_exists = _check_exists("source directory");
let source_dir = (~default=default_config.source_dir, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
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
  let resolve = (cfg: option(Config.t), root_dir) => {
    switch (cfg, value^) {
    | (_, Some(value)) =>
      let source_dir = Filename.resolve(value);
      _check_source_dir_exists(source_dir);
      source_dir |> Filename.relative_to(root_dir);
    | (Some(cfg), None) =>
      let source_dir = cfg.source_dir;
      _check_source_dir_exists(Filename.concat(root_dir, source_dir));
      source_dir;
    | (None, None) =>
      let source_dir = default;
      _check_source_dir_exists(Filename.concat(root_dir, source_dir));
      source_dir;
    };
  };

  (opt, resolve);
};

let _check_entry_exists = _check_exists(entry_key);

let entry = (~default=default_config.entry, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
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
  let resolve = (cfg: option(Config.t), root_dir, source_dir) => {
    let source_path = Filename.concat(root_dir, source_dir);

    Namespace.Internal(
      (
        switch (cfg, value^) {
        | (_, Some(value)) =>
          let entry = Filename.resolve(value);
          _check_entry_exists(entry);
          entry |> Filename.relative_to(source_path);
        | (Some(cfg), None) =>
          let entry = cfg.entry;
          _check_entry_exists(Filename.concat(source_path, entry));
          entry;
        | (None, None) =>
          let entry = default;
          _check_entry_exists(Filename.concat(source_path, entry));
          entry;
        }
      )
      |> String.drop_suffix(Constants.file_extension),
    );
  };

  (opt, resolve);
};

let target = () => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~alias="t",
      ~options=__targets,
      ~from_config=
        cfg => cfg.target |?> ~@Target.pp % (x => Opt.Value.String(x)),
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

  (opt, resolve);
};

let out_dir = (~default=default_config.out_dir, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~alias="o",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.out_dir)),
      out_dir_key,
      String(x => value := Some(x)),
      "the directory to write compiled files to",
    );
  let resolve = (cfg: option(Config.t), root_dir: string) =>
    switch (cfg, value^) {
    | (_, Some(value)) => Filename.resolve(value)
    | (Some(cfg), None) =>
      let out_dir = cfg.out_dir;

      if (Filename.is_relative(out_dir)) {
        Filename.concat(root_dir, out_dir);
      } else {
        out_dir;
      };
    | (None, None) => Filename.concat(root_dir, default)
    };

  (opt, resolve);
};

let fix = (~default=default_config.fix, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fix)),
      fix_key,
      Unit(() => value := Some(true)),
      "automatically apply fixes",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.fix, default);

  (opt, resolve);
};

let color = (~default=default_config.color, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.color)),
      color_key,
      Bool(x => value := Some(x)),
      "allow color in logs",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.color, default);

  (opt, resolve);
};

let fail_fast = (~default=default_config.fail_fast, ()) => {
  let value = ref(None);
  let opt =
    Opt.create(
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fail_fast)),
      fail_fast_key,
      Unit(() => value := Some(true)),
      "fail as soon as the first error is encountered",
    );
  let resolve = (cfg: option(Config.t)) =>
    value^ |> _resolve(cfg, x => x.fail_fast, default);

  (opt, resolve);
};
