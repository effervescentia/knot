open Kore;

type t = {
  entry: string,
  root_dir: string,
  debug: bool,
};

let from_args = (): t => {
  let debug = ref(false);
  let entry = ref("");

  Arg.parse(
    [("-debug", Arg.Set(debug), " Enable a higher level of logging")],
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
      entry: Filename.basename(entry^),
      root_dir: Filename.dirname(entry^),
      debug: debug^,
    };
  };
};
