open Kore;

type t = {
  mode: mode_t,
  entry: string,
  root_dir: string,
  debug: bool,
};

let from_args = (): t => {
  let debug = ref(false);
  let watch = ref(false);
  let entry = ref("");

  Arg.parse(
    [
      ("-debug", Arg.Set(debug), " Enable a higher level of logging"),
      (
        "-watch",
        Arg.Set(watch),
        " Watch the root directory for file changes",
      ),
    ],
    x =>
      if (entry^ == "") {
        entry := Filename.normalize(x);
      } else {
        raise(Arg.Bad(Print.fmt("unexpected argument: %s", x)));
      },
    "knotc <entry>",
  );

  if (entry^ == "") {
    raise(Arg.Bad("missing argument: entry"));
  } else {
    {
      mode: watch^ ? Watch : Static,
      entry: Filename.basename(entry^),
      root_dir: Filename.dirname(entry^),
      debug: debug^,
    };
  };
};
