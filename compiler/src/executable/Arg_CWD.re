open Kore;

let create = (~default=Sys.getcwd(), ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~default=String(default),
      cwd_key,
      String(x => value := Some(x)),
      "the working directory to execute knot commands in",
    );
  let resolve = () => value^ |?> Filename.resolve(~cwd=default);

  (argument, resolve);
};
