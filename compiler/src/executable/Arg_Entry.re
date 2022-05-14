open Kore;

let create = (~default=ConfigFile.defaults.entry, ()) => {
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
        "source dir",
      ),
    );
  let resolve = (cfg: option(Config.t), source_dir: string) => {
    let absolute_entry =
      value^
      |> Argument.resolve(cfg, x => x.entry, default)
      |> Filename.resolve(~cwd=source_dir);

    absolute_entry |> Util.assert_exists(entry_key);

    if (!String.starts_with(source_dir, absolute_entry)) {
      Fmt.str(
        "entry must be a path within the source directory %a, but found %a",
        Fmt.bold_str,
        source_dir,
        Fmt.bold_str,
        absolute_entry,
      )
      |> panic;
    };

    absolute_entry
    |> Filename.relative_to(source_dir)
    |> String.drop_suffix(Constants.file_extension)
    |> Namespace.of_internal;
  };

  (argument, resolve);
};
