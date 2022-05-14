open Kore;

let create = (~default=ConfigFile.defaults.source_dir, ()) => {
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
        "root dir",
      ),
    );
  let resolve = (cfg: option(Config.t), root_dir: string) => {
    let absolute_source_dir =
      value^
      |> Argument.resolve(cfg, x => x.source_dir, default)
      |> Filename.resolve(~cwd=root_dir);

    absolute_source_dir |> Util.assert_exists("source directory");

    if (!String.starts_with(root_dir, absolute_source_dir)) {
      Fmt.str(
        "source directory must be within root directory %a but found %a",
        Fmt.bold_str,
        root_dir,
        Fmt.bold_str,
        absolute_source_dir,
      )
      |> panic;
    };

    {
      absolute: absolute_source_dir,
      relative: absolute_source_dir |> Filename.relative_to(root_dir),
    };
  };

  (argument, resolve);
};
