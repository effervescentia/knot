open Kore;

let create = (~default=ConfigFile.defaults.working_dir, ()) => {
  let value = ref(default);
  let argument =
    Argument.create(
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.working_dir)),
      cwd_key,
      String(x => value := x),
      "the working directory to execute knot commands in",
    );
  let resolve = () => value^;

  (argument, resolve);
};
